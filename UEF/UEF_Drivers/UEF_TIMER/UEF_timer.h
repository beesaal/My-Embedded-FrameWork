/*
Add to UEF_timmer.h
void UEF_Timer_DelayMs(uint32_t ms);
void UEF_Timer_Init(void);

*/

// ============================================================
// File: UEF/UEF_Drivers/UEF_TIMER/UEF_timer.h
// ============================================================
// PURPOSE: Universal Timer driver interface
//          Works on ANY MCU through ChipLink.h
//          NO HARDWARE-SPECIFIC CODE
// ============================================================

#ifndef UEF_TIMER_H
#define UEF_TIMER_H

#include <stdint.h>
#include <stdbool.h>
#include "ChipLink.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// Timer Configuration Types
// ============================================================

typedef enum {
    UEF_TIMER_MODE_ONE_SHOT = 0,
    UEF_TIMER_MODE_PERIODIC = 1,
    UEF_TIMER_MODE_PWM = 2,
    UEF_TIMER_MODE_INPUT_CAPTURE = 3,
    UEF_TIMER_MODE_OUTPUT_COMPARE = 4
} UEF_Timer_Mode_t;

typedef struct {
    uint8_t instance;           // Timer number (2, 3, 4, 5, 9, 10, 11)
    UEF_Timer_Mode_t mode;
    uint32_t prescaler;         // Prescaler value
    uint32_t period;            // Period (auto-reload value)
    uint8_t channel;            // Channel for PWM/Compare (1-4)
    uint32_t compare_value;     // Compare value for PWM/Compare
    bool enable_interrupts;
} UEF_Timer_Config_t;

// Forward declaration
typedef struct UEF_Timer_Handle UEF_Timer_Handle_t;

struct UEF_Timer_Handle {
    void* instance;             // Hardware instance
    uint8_t instance_num;
    bool is_initialized;
    UEF_Timer_Config_t config;
    
    // State
    volatile bool busy;
    volatile uint32_t counter;
    volatile uint32_t error_flags;
    
    // Callbacks
    void (*overflow_cb)(UEF_Timer_Handle_t* handle);
    void (*capture_cb)(UEF_Timer_Handle_t* handle, uint32_t value);
    void (*error_cb)(UEF_Timer_Handle_t* handle, uint32_t error);
};

// ============================================================
// Timer API - UNIVERSAL FUNCTIONS
// ============================================================

// Initialization
int UEF_Timer_Init(UEF_Timer_Handle_t* handle, const UEF_Timer_Config_t* config);
int UEF_Timer_DeInit(UEF_Timer_Handle_t* handle);

// Control
void UEF_Timer_Start(UEF_Timer_Handle_t* handle);
void UEF_Timer_Stop(UEF_Timer_Handle_t* handle);
void UEF_Timer_Reset(UEF_Timer_Handle_t* handle);
void UEF_Timer_SetCounter(UEF_Timer_Handle_t* handle, uint32_t value);
uint32_t UEF_Timer_GetCounter(UEF_Timer_Handle_t* handle);

// PWM
void UEF_Timer_SetDutyCycle(UEF_Timer_Handle_t* handle, uint32_t duty);
void UEF_Timer_SetFrequency(UEF_Timer_Handle_t* handle, uint32_t frequency);
void UEF_Timer_SetPeriod(UEF_Timer_Handle_t* handle, uint32_t period);

// Status
bool UEF_Timer_IsRunning(UEF_Timer_Handle_t* handle);
uint32_t UEF_Timer_GetErrorFlags(UEF_Timer_Handle_t* handle);
void UEF_Timer_ClearErrorFlags(UEF_Timer_Handle_t* handle);

// Callback registration
void UEF_Timer_RegisterCallbacks(UEF_Timer_Handle_t* handle,
                                void (*overflow_cb)(UEF_Timer_Handle_t*),
                                void (*capture_cb)(UEF_Timer_Handle_t*, uint32_t),
                                void (*error_cb)(UEF_Timer_Handle_t*, uint32_t));

// Interrupt handler
void UEF_Timer_IRQHandler(UEF_Timer_Handle_t* handle);

#ifdef __cplusplus
}
#endif

#endif // UEF_TIMER_H