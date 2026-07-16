// ============================================================
// File: UEF/UEF_Drivers/UEF_CLOCK/UEF_clock.c
// ============================================================
// PURPOSE: Universal clock driver implementation
//          Uses ONLY ChipLink.h - 100% portable
//          NO HARDWARE-SPECIFIC CODE
// ============================================================

#include "UEF_clock.h"
#include "ChipLink.h"

// ============================================================
// TICK COUNTING
// ============================================================

static volatile uint32_t system_tick = 0;

void SysTick_Handler(void) {
    system_tick++;
}

uint32_t UEF_Clock_GetTick(void) {
    return system_tick;
}

// ============================================================
// PUBLIC FUNCTIONS - Universal Clock API
// ============================================================

int UEF_Clock_ConfigHSI(void) {
    return UEF_ChipLink_Clock_InitHSI();
}

int UEF_Clock_ConfigHSE(void) {
    return UEF_ChipLink_Clock_InitHSE();
}

int UEF_Clock_ConfigPLL(uint32_t hse_freq) {
    return UEF_ChipLink_Clock_InitPLL(hse_freq);
}

int UEF_Clock_ConfigSysTick(uint32_t sysclk, uint32_t ticks_per_second) {
    return UEF_ChipLink_Clock_ConfigSysTick(sysclk, ticks_per_second);
}

uint32_t UEF_Clock_GetSystemFreq(void) {
    return UEF_ChipLink_Clock_GetSystemFreq();
}

uint32_t UEF_Clock_GetHCLKFreq(void) {
    return UEF_ChipLink_Clock_GetHCLKFreq();
}

uint32_t UEF_Clock_GetAPB1Freq(void) {
    return UEF_ChipLink_Clock_GetAPB1Freq();
}

uint32_t UEF_Clock_GetAPB2Freq(void) {
    return UEF_ChipLink_Clock_GetAPB2Freq();
}

void UEF_Clock_GetConfig(UEF_ClockConfig_t* config) {
    if (!config) return;
    config->frequency = UEF_Clock_GetSystemFreq();
    config->hclk = UEF_Clock_GetHCLKFreq();
    config->apb1 = UEF_Clock_GetAPB1Freq();
    config->apb2 = UEF_Clock_GetAPB2Freq();
    // Source detection is chip-specific and handled in ChipLink
    config->source = UEF_CLOCK_SRC_HSI;  // Default
}

void UEF_DelayMs(uint32_t ms) {
    uint32_t start = UEF_Clock_GetTick();
    while ((UEF_Clock_GetTick() - start) < ms) {
        // Wait
    }
}

void UEF_DelayUs(uint32_t us) {
    uint32_t cycles_per_us = UEF_Clock_GetSystemFreq() / 1000000;
    UEF_ChipLink_Delay_Cycles(us * cycles_per_us);
}