// ============================================================
// File: UEF/UEF_Drivers/UEF_ADC/UEF_adc.c
// ============================================================
// PURPOSE: Universal ADC driver implementation
//          Uses ONLY ChipLink.h - 100% portable
//          NO HARDWARE-SPECIFIC CODE
// ============================================================

#include "UEF_adc.h"
#include "ChipLink.h"

// ============================================================
// PRIVATE HELPERS - Call ONLY ChipLink functions
// ============================================================

static void UEF_ADC_Calibrate(UEF_ADC_Handle_t* handle) {
    void* adc = handle->instance;
    
    // Enable ADC via ChipLink
    UEF_ADC_ENABLE(adc);
    
    // Wait for ADC to stabilize
    for (volatile int i = 0; i < 1000; i++);
    
    // Start calibration (if supported) - handled by ChipLink
}

// ============================================================
// INITIALIZATION
// ============================================================

int UEF_ADC_Init(UEF_ADC_Handle_t* handle, const UEF_ADC_Config_t* config) {
    if (!handle || !config) return -1;
    if (config->instance == 0) return -2;

    // Get hardware instance via ChipLink
    void* adc = UEF_ADC_GET(config->instance);
    if (!adc) return -3;

    // Store config
    handle->instance = adc;
    handle->instance_num = config->instance;
    handle->config = *config;
    handle->is_initialized = false;
    handle->busy = false;
    handle->last_value = 0;
    handle->error_flags = 0;
    handle->conv_complete_cb = NULL;
    handle->error_cb = NULL;

    // Enable clock via ChipLink
    UEF_ADC_CLK_ENABLE(config->instance);

    // Configure pin as analog via ChipLink
    if (config->gpio_port) {
        UEF_ADC_ConfigurePin(config->gpio_port, config->gpio_pin);
    }

    // Disable ADC for configuration via ChipLink
    UEF_ADC_DISABLE(adc);

    // Configure resolution via ChipLink
    UEF_ADC_SET_RESOLUTION(adc, config->resolution);

    // Configure alignment via ChipLink
    UEF_ADC_SET_ALIGN(adc, config->align == UEF_ADC_ALIGN_LEFT);

    // Configure channel via ChipLink
    UEF_ADC_SET_CHANNEL(adc, config->channel);

    // Configure sample time via ChipLink
    UEF_ADC_SET_SAMPLE_TIME(adc, config->channel, config->sample_time);

    // Calibrate via ChipLink
    UEF_ADC_Calibrate(handle);

    // Enable interrupts if requested via ChipLink
    if (config->enable_interrupts) {
        UEF_ADC_ENABLE_INTERRUPT(adc);
    }

    handle->is_initialized = true;
    return 0;
}

int UEF_ADC_DeInit(UEF_ADC_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return -1;

    void* adc = handle->instance;

    // Disable ADC via ChipLink
    UEF_ADC_DISABLE(adc);

    // Disable clock via ChipLink
    UEF_ADC_CLK_DISABLE(handle->instance_num);

    handle->is_initialized = false;
    return 0;
}

// ============================================================
// CONVERSION FUNCTIONS
// ============================================================

int UEF_ADC_StartConversion(UEF_ADC_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return -1;
    if (handle->busy) return -2;

    void* adc = handle->instance;
    handle->busy = true;

    // Start conversion via ChipLink
    UEF_ADC_START_CONVERSION(adc);

    // Wait for conversion to complete via ChipLink
    while (!UEF_ADC_CONVERSION_DONE(adc));

    // Read value via ChipLink
    handle->last_value = UEF_ADC_GET_VALUE(adc);

    handle->busy = false;

    if (handle->conv_complete_cb) {
        handle->conv_complete_cb(handle, handle->last_value);
    }

    return 0;
}

int UEF_ADC_StartConversion_IT(UEF_ADC_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return -1;
    if (handle->busy) return -2;

    void* adc = handle->instance;
    handle->busy = true;

    // Enable interrupt and start conversion via ChipLink
    UEF_ADC_ENABLE_INTERRUPT(adc);
    UEF_ADC_START_CONVERSION(adc);

    return 0;
}

uint32_t UEF_ADC_GetValue(UEF_ADC_Handle_t* handle) {
    if (!handle) return 0;
    return handle->last_value;
}

bool UEF_ADC_IsConversionComplete(UEF_ADC_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return false;
    return UEF_ADC_CONVERSION_DONE(handle->instance);
}

// ============================================================
// UTILITY FUNCTIONS
// ============================================================

uint16_t UEF_ADC_ReadChannel(UEF_ADC_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return 0;
    
    if (UEF_ADC_StartConversion(handle) != 0) {
        return 0;
    }
    
    return (uint16_t)handle->last_value;
}

float UEF_ADC_GetVoltage(UEF_ADC_Handle_t* handle, float vref) {
    if (!handle || !handle->is_initialized) return 0.0f;
    
    uint16_t value = UEF_ADC_ReadChannel(handle);
    uint32_t max_value = 0xFFF;  // 12-bit max
    
    return (value * vref) / max_value;
}

// ============================================================
// STATUS FUNCTIONS
// ============================================================

bool UEF_ADC_IsBusy(UEF_ADC_Handle_t* handle) {
    if (!handle) return false;
    return handle->busy;
}

uint32_t UEF_ADC_GetErrorFlags(UEF_ADC_Handle_t* handle) {
    if (!handle) return 0;
    return handle->error_flags;
}

void UEF_ADC_ClearErrorFlags(UEF_ADC_Handle_t* handle) {
    if (!handle) return;
    handle->error_flags = 0;
}

// ============================================================
// CALLBACK REGISTRATION
// ============================================================

void UEF_ADC_RegisterCallback(UEF_ADC_Handle_t* handle,
                             void (*conv_cb)(UEF_ADC_Handle_t*, uint32_t)) {
    if (handle) {
        handle->conv_complete_cb = conv_cb;
    }
}

// ============================================================
// INTERRUPT HANDLER
// ============================================================

void UEF_ADC_IRQHandler(UEF_ADC_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;
    
    void* adc = handle->instance;
    
    if (UEF_ADC_CONVERSION_DONE(adc)) {
        handle->last_value = UEF_ADC_GET_VALUE(adc);
        handle->busy = false;
        
        // Disable interrupt via ChipLink
        UEF_ADC_DISABLE_INTERRUPT(adc);
        
        if (handle->conv_complete_cb) {
            handle->conv_complete_cb(handle, handle->last_value);
        }
    }
}