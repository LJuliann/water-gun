#include "FreeRTOS.h"
#include "gun_utils.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "queue.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// 0: Pico stdio (USB or UART). 1: UART0 with RX interrupts.
#ifndef USE_UART_IRQ
#define USE_UART_IRQ 0
#endif

#if USE_UART_IRQ
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "stream_buffer.h"
#endif

// V1 / Wokwi wiring. Change these four GPIOs to match the physical board.
// Drone gun: pitch servo on GP16, yaw servo on GP12 (verified on the assembly).
#define PITCH_SERVO_PIN 16
#define YAW_SERVO_PIN 12
#define PUMP_EN_PIN 8 // Pump enable (active high)
#define PUMP_PH_PIN 9 // Pump direction: V1 uses PH=0.
// Wokwi centers. Calibrate these values against the mechanical assembly.
#define PITCH_SERVO_CENTER_DEG 90
#define YAW_SERVO_CENTER_DEG 90
// Servo direction (+1 or -1) so the commands follow the drone frame used by
// the Jetson (ROS REP-103): positive pitch raises the barrel, positive yaw
// turns it to the drone's left. Both servos turn the other way on the drone.
#define PITCH_SERVO_DIRECTION (-1)
#define YAW_SERVO_DIRECTION (-1)
#define SERIAL_BUFFER_SIZE 64

static QueueHandle_t aim_queue;
static QueueHandle_t shoot_queue;
// Protected by task critical sections; includes an accepted but not yet started shot.
static bool shoot_busy;

void init_servo(uint pin) {
  gpio_set_function(pin, GPIO_FUNC_PWM);
  uint slice = pwm_gpio_to_slice_num(pin);
  uint32_t div = clock_get_hz(clk_sys) / 1000000u;

  pwm_set_clkdiv_int_frac4(slice, div, 0);
  pwm_set_wrap(slice, 19999);
  pwm_set_gpio_level(pin, 1500);
  pwm_set_enabled(slice, true);
}

void set_servo_angle(uint pin, int angle) {
  if (angle < 0)
    angle = 0;
  if (angle > 180)
    angle = 180;
  uint level = 1000u + (1000u * (uint)angle + 90u) / 180u;
  pwm_set_gpio_level(pin, level);
}

#if USE_UART_IRQ
static StreamBufferHandle_t uart_rx_buffer;
static volatile bool uart_overflow = false;

static void uart_rx_irq(void) {
  BaseType_t woken = pdFALSE;

  while (uart_is_readable(uart0)) {
    uint8_t byte = (uint8_t)uart_getc(uart0);
    if (xStreamBufferSendFromISR(uart_rx_buffer, &byte, 1, &woken) != 1) {
      uart_overflow = true;
    }
  }
  portYIELD_FROM_ISR(woken);
}

static void init_uart(void) {
  uart_init(uart0, 115200);
  gpio_set_function(0, GPIO_FUNC_UART); // TX
  gpio_set_function(1, GPIO_FUNC_UART); // RX

  irq_set_exclusive_handler(UART0_IRQ, uart_rx_irq);
  irq_set_priority(UART0_IRQ, 0xC0); // Low-priority IRQ: FreeRTOS-safe.
  irq_set_enabled(UART0_IRQ, true);
  uart_set_irqs_enabled(uart0, true, false);
}

static size_t receive_uart(uint8_t *data, size_t size) {
  return xStreamBufferReceive(uart_rx_buffer, data, size, portMAX_DELAY);
}
#else
static size_t receive_stdio(uint8_t *data, size_t size) {
  size_t count = 0;

  while (count < size) {
    int c = getchar_timeout_us(0);
    if (c < 0)
      break;
    data[count++] = (uint8_t)c;
  }

  if (count == 0)
    vTaskDelay(1);
  return count;
}
#endif

void task_serial(void *params) {
  (void)params;

#if USE_UART_IRQ
  init_uart();
#endif

  char line[SERIAL_BUFFER_SIZE];
  uint8_t data[32];
  size_t index = 0;
  bool discard = false;

  while (true) {
#if USE_UART_IRQ
    size_t count = receive_uart(data, sizeof(data));
    if (uart_overflow) {
      discard = true; // Ignore a potentially corrupted command.
      uart_overflow = false;
    }
#else
    size_t count = receive_stdio(data, sizeof(data));
#endif

    for (size_t i = 0; i < count; i++) {
      uint8_t c = data[i];

      if (c == '\n' || c == '\r') {
        if (!discard && index > 0) {
          line[index] = '\0';
          GunCommand command;
          if (decode_command(line, &command)) {
            if (command.type == CMD_AIM) {
              xQueueOverwrite(aim_queue, &command);
            } else if (command.type == CMD_SHOOT) {
              BaseType_t accepted = pdFALSE;
              taskENTER_CRITICAL();
              if (!shoot_busy) {
                shoot_busy = true;
                accepted = xQueueSend(shoot_queue, &command, 0);
                if (accepted != pdTRUE)
                  shoot_busy = false;
              }
              taskEXIT_CRITICAL();
#if defined(DEBUG) && DEBUG
              if (accepted != pdTRUE)
                printf("SHOOT: busy\n");
#endif
            }
          } else {
#if defined(DEBUG) && DEBUG
            printf("Invalid command\n");
#endif
          }
        }
#if defined(DEBUG) && DEBUG
        if (discard) {
          printf("Serial line discarded\n");
        }
#endif
        index = 0;
        discard = false;
      } else if (!discard) {
        if (c == 0 || index >= sizeof(line) - 1) {
          discard = true;
        } else {
          line[index++] = (char)c;
        }
      }
    }
  }
}

void task_aim(void *params) {
  (void)params;
  GunCommand command;

  while (true) {
    if (xQueueReceive(aim_queue, &command, portMAX_DELAY) == pdTRUE) {
      set_servo_angle(PITCH_SERVO_PIN, PITCH_SERVO_CENTER_DEG +
                                           PITCH_SERVO_DIRECTION * command.pitch);
      set_servo_angle(YAW_SERVO_PIN,
                      YAW_SERVO_CENTER_DEG + YAW_SERVO_DIRECTION * command.yaw);
#if defined(DEBUG) && DEBUG
      printf("AIM: pitch=%d yaw=%d\n", command.pitch, command.yaw);
#endif
    }
  }
}

static int64_t pump_cutoff(alarm_id_t id, void *user_data) {
  (void)id;
  (void)user_data;
  gpio_put(PUMP_EN_PIN, 0);
  return 0;
}

void task_shoot(void *params) {
  (void)params;
  GunCommand command;

  while (true) {
    if (xQueueReceive(shoot_queue, &command, portMAX_DELAY) == pdTRUE) {
      alarm_id_t cutoff = -1;
      if (command.duration_ms >= 500 && command.duration_ms <= 2000) {
        // Arm the IRQ cutoff before enabling the pump. An expired/failed alarm
        // must never be followed by an unprotected pump start.
        uint32_t irq_state = save_and_disable_interrupts();
        cutoff = add_alarm_in_ms(command.duration_ms, pump_cutoff, NULL, false);
        if (cutoff > 0) {
          gpio_put(PUMP_PH_PIN, 0);
          gpio_put(PUMP_EN_PIN, 1);
        }
        restore_interrupts(irq_state);
      }

      if (cutoff > 0) {
#if defined(DEBUG) && DEBUG
        printf("SHOOT: duration=%d ms\n", command.duration_ms);
#endif
        vTaskDelay(pdMS_TO_TICKS(command.duration_ms));
      }
      gpio_put(PUMP_EN_PIN, 0);
      if (cutoff > 0)
        cancel_alarm(cutoff);
      taskENTER_CRITICAL();
      shoot_busy = false;
      taskEXIT_CRITICAL();
#if defined(DEBUG) && DEBUG
      printf("SHOOT: %s\n", cutoff > 0 ? "done" : "refused (duration/cutoff)");
#endif
    }
  }
}

int main(void) {
  // Hold the pump off before starting any task. PH=0 is the V1 direction.
  gpio_init(PUMP_EN_PIN);
  gpio_put(PUMP_EN_PIN, 0);
  gpio_set_dir(PUMP_EN_PIN, GPIO_OUT);
  gpio_init(PUMP_PH_PIN);
  gpio_put(PUMP_PH_PIN, 0);
  gpio_set_dir(PUMP_PH_PIN, GPIO_OUT);

  stdio_init_all();
  init_servo(PITCH_SERVO_PIN);
  init_servo(YAW_SERVO_PIN);
  set_servo_angle(PITCH_SERVO_PIN, PITCH_SERVO_CENTER_DEG);
  set_servo_angle(YAW_SERVO_PIN, YAW_SERVO_CENTER_DEG);

  aim_queue = xQueueCreate(1, sizeof(GunCommand));
  // Handoff only: shoot_busy rejects further requests until this shot finishes.
  shoot_queue = xQueueCreate(1, sizeof(GunCommand));
  if (!aim_queue || !shoot_queue)
    while (true)
      tight_loop_contents();

#if USE_UART_IRQ
  uart_rx_buffer = xStreamBufferCreate(128, 1);
  if (!uart_rx_buffer)
    while (true)
      tight_loop_contents();
#endif

  if (xTaskCreate(task_serial, "Serial", 1024, NULL, 2, NULL) != pdPASS ||
      xTaskCreate(task_aim, "Aim", 1024, NULL, 2, NULL) != pdPASS ||
      xTaskCreate(task_shoot, "Shoot", 1024, NULL, 2, NULL) != pdPASS) {
    while (true)
      tight_loop_contents();
  }

#if defined(DEBUG) && DEBUG
  printf("Ready: serial -> aim/shoot (%s)\n",
         USE_UART_IRQ ? "UART0 IRQ" : "stdio");
#endif
  vTaskStartScheduler();
  while (true)
    tight_loop_contents();
}
