#ifndef WATER_GUN_FREERTOS_CONFIG_H
#define WATER_GUN_FREERTOS_CONFIG_H

#include <assert.h>

/* Ordonnanceur sur un seul coeur ; un tick vaut 1 ms. */
#define configNUMBER_OF_CORES                    1
#define configTICK_CORE                          0
#define configUSE_PREEMPTION                     1
#define configUSE_TIME_SLICING                   1
#define configUSE_TICKLESS_IDLE                  0
#define configTICK_RATE_HZ                       1000U
#define configTICK_TYPE_WIDTH_IN_BITS            TICK_TYPE_WIDTH_32_BITS
#define configMAX_PRIORITIES                     5
#define configMINIMAL_STACK_SIZE                 512U
#define configMAX_TASK_NAME_LEN                  16

/* xTaskCreate() et l'allocateur heap_4 du noyau. Les piles sont en mots. */
#define configSUPPORT_DYNAMIC_ALLOCATION         1
#define configSUPPORT_STATIC_ALLOCATION          0
#define configTOTAL_HEAP_SIZE                    (32U * 1024U)
#define configUSE_MUTEXES                        1
#define configUSE_RECURSIVE_MUTEXES              1
#define configUSE_COUNTING_SEMAPHORES            1
#define configUSE_TIMERS                         1
#define configTIMER_TASK_PRIORITY                (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                 10
#define configTIMER_TASK_STACK_DEPTH             1024U
#define configUSE_IDLE_HOOK                      0
#define configUSE_TICK_HOOK                      0

/* Options requises par le port RP2350 ARM, en mode Secure sans TrustZone. */
#define configENABLE_MPU                         0
#define configENABLE_TRUSTZONE                   0
#define configRUN_FREERTOS_SECURE_ONLY           1
#define configENABLE_FPU                         1
#define configMAX_SYSCALL_INTERRUPT_PRIORITY     16

/* Compatibilite des primitives de synchronisation et de temps du Pico SDK. */
#define configSUPPORT_PICO_SYNC_INTEROP          1
#define configSUPPORT_PICO_TIME_INTEROP          1
#define INCLUDE_vTaskDelay                       1
#define INCLUDE_xTaskGetCurrentTaskHandle        1
#define INCLUDE_xTimerPendFunctionCall           1

#define configASSERT(condition)                 assert(condition)

#endif
