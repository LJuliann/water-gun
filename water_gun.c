
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "stdio.h"

// Servo parameters
#define WRAP 19999
#define SERVO_PIN_12 12

#define SERVO_MIN_LEVEL 1000
#define SERVO_MAX_LEVEL 2000

#define SERVO_STEP 1
#define SERVO_DELAY_MS 20

void init_servo(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Get actual system clock frequency
    uint32_t sys_clk = clock_get_hz(clk_sys);

    // 1 PWM count = 1 microsecond
    // Divider in sixteenths; keep the calculation independent of the FPU.
    uint32_t clkdiv16 = (sys_clk * 16u + 500000u) / 1000000u;
    pwm_set_clkdiv_int_frac4(slice_num, clkdiv16 / 16u, clkdiv16 % 16u);
    pwm_set_wrap(slice_num, WRAP);

    // Initial position: 90 degrees
    pwm_set_gpio_level(pin, 1500);

    pwm_set_enabled(slice_num, true);
}

// Change servo position
void set_servo_angle(uint pin, int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    // Rounded pulse width in microseconds, using integer arithmetic.
    uint level = SERVO_MIN_LEVEL + ((SERVO_MAX_LEVEL - SERVO_MIN_LEVEL) * (uint)angle + 90u) / 180u;

    pwm_set_gpio_level(pin, level);
}

int main(void)
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    stdio_init_all();
    printf("Demarrage water_gun : servo sur GP12, PWM 50 Hz\n");
    init_servo(SERVO_PIN_12);

    int cycle = 0;

    while (true) {
        printf("Cycle %d\n", cycle++);

        for (int angle = 0; angle <= 180; angle += SERVO_STEP) {
            set_servo_angle(SERVO_PIN_12, angle);
            gpio_put(PICO_DEFAULT_LED_PIN, (angle / 15) % 2);
            if (angle % 30 == 0) printf("Aller : %d deg\n", angle);
            sleep_ms(SERVO_DELAY_MS);
        }

        for (int angle = 180; angle >= 0; angle -= SERVO_STEP) {
            set_servo_angle(SERVO_PIN_12, angle);
            gpio_put(PICO_DEFAULT_LED_PIN, (angle / 15) % 2);
            if (angle % 30 == 0) printf("Retour : %d deg\n", angle);
            sleep_ms(SERVO_DELAY_MS);
        }
    }
}
