// ============================================================
// File: UEF/UEF_Drivers/UEF_TIMER/UEF_timer.c
// ============================================================
// PURPOSE: Universal Timer driver implementation
//          Uses ONLY ChipLink.h - 100% portable
//          NO HARDWARE-SPECIFIC CODE
// ============================================================

#include "UEF_timer.h"
#include "ChipLink.h"

// ============================================================
// PRIVATE HELPERS - Call ONLY ChipLink functions
// ============================================================

static uint32_t UEF_Timer_GetClockFreq(uint8_t instance) {
    return UEF_TIMER_GetClockFreq(instance);
}

// ============================================================
// INITIALIZATION
// ============================================================

int UEF_Timer_Init(UEF_Timer_Handle_t* handle, const UEF_Timer_Config_t* config) {
    if (!handle || !config) return -1;
    if (config->instance == 0) return -2;

    // Get hardware instance via ChipLink
    void* tim = UEF_TIMER_GET(config->instance);
    if (!tim) return -3;

    // Store config
    handle->instance = tim;
    handle->instance_num = config->instance;
    handle->config = *config;
    handle->is_initialized = false;
    handle->busy = false;
    handle->counter = 0;
    handle->error_flags = 0;
    handle->overflow_cb = NULL;
    handle->capture_cb = NULL;
    handle->error_cb = NULL;

    // Enable clock via ChipLink
    UEF_TIMER_CLK_ENABLE(config->instance);

    // Disable timer for configuration via ChipLink
    UEF_TIMER_STOP(tim);

    // Configure prescaler via ChipLink
    UEF_TIMER_SET_PRESCALER(tim, config->prescaler);

    // Configure period via ChipLink
    UEF_TIMER_SET_PERIOD(tim, config->period);

    // Enable ARR preload via ChipLink
    UEF_TIMER_ENABLE_ARR_PRELOAD(tim);

    // Configure PWM mode if needed via ChipLink
    if (config->mode == UEF_TIMER_MODE_PWM) {
        UEF_TIMER_SET_PWM_MODE(tim, config->channel);
        UEF_TIMER_SET_COMPARE(tim, config->channel, config->compare_value);
        UEF_TIMER_ENABLE_CHANNEL_OUTPUT(tim, config->channel);
        UEF_TIMER_ENABLE_MAIN_OUTPUT(tim);
    }

    // Enable interrupts if requested via ChipLink
    if (config->enable_interrupts) {
        UEF_TIMER_ENABLE_UPDATE_INTERRUPT(tim);
        UEF_TIMER_ENABLE_IRQ(config->instance);
    }

    handle->is_initialized = true;
    return 0;
}

int UEF_Timer_DeInit(UEF_Timer_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return -1;

    void* tim = handle->instance;

    // Disable timer via ChipLink
    UEF_TIMER_STOP(tim);

    // Disable interrupts via ChipLink
    UEF_TIMER_DISABLE_UPDATE_INTERRUPT(tim);
    UEF_TIMER_DISABLE_IRQ(handle->instance_num);

    // Disable clock via ChipLink
    UEF_TIMER_CLK_DISABLE(handle->instance_num);

    handle->is_initialized = false;
    return 0;
}

// ============================================================
// CONTROL FUNCTIONS
// ============================================================

void UEF_Timer_Start(UEF_Timer_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;
    UEF_TIMER_START(handle->instance);
}

void UEF_Timer_Stop(UEF_Timer_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;
    UEF_TIMER_STOP(handle->instance);
}

void UEF_Timer_Reset(UEF_Timer_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;
    void* tim = handle->instance;
    UEF_TIMER_SET_COUNTER(tim, 0);
    UEF_TIMER_GENERATE_UPDATE(tim);
}

void UEF_Timer_SetCounter(UEF_Timer_Handle_t* handle, uint32_t value) {
    if (!handle || !handle->is_initialized) return;
    UEF_TIMER_SET_COUNTER(handle->instance, value);
}

uint32_t UEF_Timer_GetCounter(UEF_Timer_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return 0;
    return UEF_TIMER_GET_COUNTER(handle->instance);
}

// ============================================================
// PWM FUNCTIONS
// ============================================================

void UEF_Timer_SetDutyCycle(UEF_Timer_Handle_t* handle, uint32_t duty) {
    if (!handle || !handle->is_initialized) return;
    if (handle->config.mode != UEF_TIMER_MODE_PWM) return;
    
    void* tim = handle->instance;
    uint32_t period = UEF_TIMER_GET_PERIOD(tim);
    uint32_t compare = (duty * period) / 100;
    UEF_TIMER_SET_COMPARE(tim, handle->config.channel, compare);
    handle->config.compare_value = compare;
}

void UEF_Timer_SetFrequency(UEF_Timer_Handle_t* handle, uint32_t frequency) {
    if (!handle || !handle->is_initialized) return;
    if (frequency == 0) return;
    
    uint32_t clock_freq = UEF_Timer_GetClockFreq(handle->instance_num);
    uint32_t period = clock_freq / (frequency * (handle->config.prescaler + 1));
    UEF_TIMER_SET_PERIOD(handle->instance, period);
    handle->config.period = period;
}

void UEF_Timer_SetPeriod(UEF_Timer_Handle_t* handle, uint32_t period) {
    if (!handle || !handle->is_initialized) return;
    UEF_TIMER_SET_PERIOD(handle->instance, period);
    handle->config.period = period;
}

// ============================================================
// STATUS FUNCTIONS
// ============================================================

bool UEF_Timer_IsRunning(UEF_Timer_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return false;
    // ChipLink handles the actual status check
    return handle->busy;
}

uint32_t UEF_Timer_GetErrorFlags(UEF_Timer_Handle_t* handle) {
    if (!handle) return 0;
    return handle->error_flags;
}

void UEF_Timer_ClearErrorFlags(UEF_Timer_Handle_t* handle) {
    if (!handle) return;
    handle->error_flags = 0;
}

// ============================================================
// CALLBACK REGISTRATION
// ============================================================

void UEF_Timer_RegisterCallbacks(UEF_Timer_Handle_t* handle,
                                void (*overflow_cb)(UEF_Timer_Handle_t*),
                                void (*capture_cb)(UEF_Timer_Handle_t*, uint32_t),
                                void (*error_cb)(UEF_Timer_Handle_t*, uint32_t)) {
    if (!handle) return;
    handle->overflow_cb = overflow_cb;
    handle->capture_cb = capture_cb;
    handle->error_cb = error_cb;
}

// ============================================================
// INTERRUPT HANDLER
// ============================================================

void UEF_Timer_IRQHandler(UEF_Timer_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;

    void* tim = handle->instance;

    // Check for update flag via ChipLink
    if (UEF_TIMER_IS_UPDATE_FLAG_SET(tim)) {
        UEF_TIMER_CLEAR_UPDATE_FLAG(tim);
        handle->counter = UEF_TIMER_GET_COUNTER(tim);
        if (handle->overflow_cb) {
            handle->overflow_cb(handle);
        }
    }
}