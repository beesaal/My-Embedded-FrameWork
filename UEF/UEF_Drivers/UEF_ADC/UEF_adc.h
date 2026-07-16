// ============================================================
// File: UEF/UEF_Drivers/UEF_ADC/UEF_adc.h
// ============================================================
// PURPOSE: Universal ADC driver interface
//          Works on ANY MCU through ChipLink.h
//          NO HARDWARE-SPECIFIC CODE
// ============================================================

#ifndef UEF_ADC_H
#define UEF_ADC_H

#include <stdint.h>
#include <stdbool.h>
#include "ChipLink.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// ADC Configuration Types
// ============================================================

typedef enum {
    UEF_ADC_RESOLUTION_12BIT = 0,
    UEF_ADC_RESOLUTION_10BIT = 1,
    UEF_ADC_RESOLUTION_8BIT = 2,
    UEF_ADC_RESOLUTION_6BIT = 3
} UEF_ADC_Resolution_t;

typedef enum {
    UEF_ADC_ALIGN_RIGHT = 0,
    UEF_ADC_ALIGN_LEFT = 1
} UEF_ADC_Align_t;

typedef enum {
    UEF_ADC_SAMPLE_3_CYCLES = 0,
    UEF_ADC_SAMPLE_15_CYCLES = 1,
    UEF_ADC_SAMPLE_28_CYCLES = 2,
    UEF_ADC_SAMPLE_56_CYCLES = 3,
    UEF_ADC_SAMPLE_84_CYCLES = 4,
    UEF_ADC_SAMPLE_112_CYCLES = 5,
    UEF_ADC_SAMPLE_144_CYCLES = 6,
    UEF_ADC_SAMPLE_480_CYCLES = 7
} UEF_ADC_SampleTime_t;

typedef struct {
    uint8_t instance;           // ADC number (1, 2, 3, etc.)
    uint8_t channel;            // ADC channel (0-15)
    void* gpio_port;            // GPIO port for the pin
    uint16_t gpio_pin;          // GPIO pin number
    UEF_ADC_Resolution_t resolution;
    UEF_ADC_Align_t align;
    UEF_ADC_SampleTime_t sample_time;
    bool enable_interrupts;
} UEF_ADC_Config_t;

// Forward declaration
typedef struct UEF_ADC_Handle UEF_ADC_Handle_t;

struct UEF_ADC_Handle {
    void* instance;             // Hardware instance
    uint8_t instance_num;
    bool is_initialized;
    UEF_ADC_Config_t config;
    
    // State
    volatile bool busy;
    volatile uint32_t last_value;
    volatile uint32_t error_flags;
    
    // Callbacks
    void (*conv_complete_cb)(UEF_ADC_Handle_t* handle, uint32_t value);
    void (*error_cb)(UEF_ADC_Handle_t* handle, uint32_t error);
};

// ============================================================
// ADC API - UNIVERSAL FUNCTIONS
// ============================================================

// Initialization
int UEF_ADC_Init(UEF_ADC_Handle_t* handle, const UEF_ADC_Config_t* config);
int UEF_ADC_DeInit(UEF_ADC_Handle_t* handle);

// Conversion
int UEF_ADC_StartConversion(UEF_ADC_Handle_t* handle);
int UEF_ADC_StartConversion_IT(UEF_ADC_Handle_t* handle);
uint32_t UEF_ADC_GetValue(UEF_ADC_Handle_t* handle);
bool UEF_ADC_IsConversionComplete(UEF_ADC_Handle_t* handle);

// Utility
uint16_t UEF_ADC_ReadChannel(UEF_ADC_Handle_t* handle);
float UEF_ADC_GetVoltage(UEF_ADC_Handle_t* handle, float vref);

// Status
bool UEF_ADC_IsBusy(UEF_ADC_Handle_t* handle);
uint32_t UEF_ADC_GetErrorFlags(UEF_ADC_Handle_t* handle);
void UEF_ADC_ClearErrorFlags(UEF_ADC_Handle_t* handle);

// Callback registration
void UEF_ADC_RegisterCallback(UEF_ADC_Handle_t* handle,
                             void (*conv_cb)(UEF_ADC_Handle_t*, uint32_t));

// Interrupt handler
void UEF_ADC_IRQHandler(UEF_ADC_Handle_t* handle);

#ifdef __cplusplus
}
#endif

#endif // UEF_ADC_H