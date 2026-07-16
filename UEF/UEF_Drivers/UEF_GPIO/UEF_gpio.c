// ============================================================
// File: UEF/UEF_Drivers/UEF_GPIO/UEF_gpio.c
// ============================================================
// PURPOSE: Universal GPIO driver implementation
//          Uses ONLY ChipLink.h - 100% portable
//          NO HARDWARE-SPECIFIC CODE
// ============================================================

#include "UEF_gpio.h"
#include "ChipLink.h"

// ============================================================
// PRIVATE HELPERS - Call ONLY ChipLink functions
// ============================================================

static uint32_t UEF_GPIO_GetModeValue(UEF_GPIO_Mode_t mode) {
    switch (mode) {
        case UEF_GPIO_MODE_INPUT:      return UEF_GPIO_MODE_INPUT_VAL;
        case UEF_GPIO_MODE_OUTPUT_PP:  return UEF_GPIO_MODE_OUTPUT_VAL;
        case UEF_GPIO_MODE_OUTPUT_OD:  return UEF_GPIO_MODE_OUTPUT_VAL;
        case UEF_GPIO_MODE_AF_PP:      return UEF_GPIO_MODE_AF_VAL;
        case UEF_GPIO_MODE_AF_OD:      return UEF_GPIO_MODE_AF_VAL;
        case UEF_GPIO_MODE_ANALOG:     return UEF_GPIO_MODE_ANALOG_VAL;
        default:                       return UEF_GPIO_MODE_INPUT_VAL;
    }
}

static bool UEF_GPIO_IsOpenDrain(UEF_GPIO_Mode_t mode) {
    return (mode == UEF_GPIO_MODE_OUTPUT_OD ||
            mode == UEF_GPIO_MODE_AF_OD);
}

// ============================================================
// INITIALIZATION
// ============================================================

int UEF_GPIO_Init(UEF_GPIO_Handle_t* handle, const UEF_GPIO_Config_t* config) {
    if (!handle || !config || !config->port) return -1;
    if (config->pin > 15) return -2;

    // Store configuration
    handle->port = config->port;
    handle->pin = config->pin;
    handle->config = *config;

    // Enable clock for this GPIO port via ChipLink
    UEF_GPIO_CLK_ENABLE(config->port);

    // Configure mode via ChipLink
    uint32_t mode_val = UEF_GPIO_GetModeValue(config->mode);
    UEF_GPIO_SET_MODE(config->port, config->pin, mode_val);

    // Configure output type via ChipLink
    UEF_GPIO_SET_OTYPE(config->port, config->pin, UEF_GPIO_IsOpenDrain(config->mode));

    // Configure speed via ChipLink
    UEF_GPIO_SET_SPEED(config->port, config->pin, config->speed);

    // Configure pull-up/pull-down via ChipLink
    UEF_GPIO_SET_PULL(config->port, config->pin, config->pull);

    // Configure alternate function via ChipLink (if AF mode)
    if (config->mode == UEF_GPIO_MODE_AF_PP || config->mode == UEF_GPIO_MODE_AF_OD) {
        UEF_GPIO_SET_AF(config->port, config->pin, config->alternate_function);
    }

    handle->is_initialized = true;
    return 0;
}

int UEF_GPIO_DeInit(UEF_GPIO_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return -1;

    // Disable clock via ChipLink
    UEF_GPIO_CLK_DISABLE(handle->port);

    handle->is_initialized = false;
    return 0;
}

// ============================================================
// WRITE OPERATIONS
// ============================================================

void UEF_GPIO_Write(UEF_GPIO_Handle_t* handle, bool state) {
    if (!handle || !handle->is_initialized) return;
    UEF_GPIO_WRITE(handle->port, handle->pin, state);
}

void UEF_GPIO_Set(UEF_GPIO_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;
    UEF_GPIO_WRITE(handle->port, handle->pin, true);
}

void UEF_GPIO_Reset(UEF_GPIO_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;
    UEF_GPIO_WRITE(handle->port, handle->pin, false);
}

void UEF_GPIO_Toggle(UEF_GPIO_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;
    UEF_GPIO_TOGGLE(handle->port, handle->pin);
}

// ============================================================
// READ OPERATIONS
// ============================================================

bool UEF_GPIO_Read(UEF_GPIO_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return false;
    return UEF_GPIO_READ(handle->port, handle->pin);
}

// ============================================================
// CONFIGURATION
// ============================================================

void UEF_GPIO_SetMode(UEF_GPIO_Handle_t* handle, UEF_GPIO_Mode_t mode) {
    if (!handle || !handle->is_initialized) return;

    uint32_t mode_val = UEF_GPIO_GetModeValue(mode);
    UEF_GPIO_SET_MODE(handle->port, handle->pin, mode_val);
    handle->config.mode = mode;
}

void UEF_GPIO_SetPull(UEF_GPIO_Handle_t* handle, UEF_GPIO_Pull_t pull) {
    if (!handle || !handle->is_initialized) return;

    UEF_GPIO_SET_PULL(handle->port, handle->pin, pull);
    handle->config.pull = pull;
}

// ============================================================
// PORT OPERATIONS
// ============================================================

void UEF_GPIO_WritePort(void* port, uint32_t value) {
    if (!port) return;
    UEF_GPIO_WRITE_PORT(port, value);
}

uint32_t UEF_GPIO_ReadPort(void* port) {
    if (!port) return 0;
    return UEF_GPIO_READ_PORT(port);
}