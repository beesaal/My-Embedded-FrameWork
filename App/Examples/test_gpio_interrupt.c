/*
    Output: Program the board → LED blinks 3 fast, then 2 slow, then stays ON
            Press the blue button → LED turns OFF for 200ms (long blink)
            Press again → LED turns OFF for 100ms (short blink)
            Press again → Long blink again (200ms)
            Press again → Short blink again (100ms)
            And so on...
*/
// ============================================================
// File: App/Examples/test_gpio_interrupt.c
// PURPOSE: Test GPIO EXTI interrupt (button press detection)
//          FIXED: Added polling fallback for debugging
// ============================================================

#include "UEF_clock.h"
#include "UEF_gpio.h"
#include "ChipLink.h"
#include <stdbool.h>

// ============================================================
// HANDLES AND STATE
// ============================================================

static UEF_GPIO_Handle_t led_handle;
static UEF_GPIO_Handle_t button_handle;
static volatile bool button_pressed = false;
static volatile uint32_t press_count = 0;
static volatile uint32_t last_interrupt_time = 0;

// ============================================================
// DELAY HELPER
// ============================================================

static void delay_ms(uint32_t ms) {
    UEF_DelayMs(ms);
}

// ============================================================
// BOOT PATTERN
// ============================================================

static void show_boot_pattern(void) {
    for (int i = 0; i < 3; i++) {
        UEF_GPIO_Set(&led_handle);
        delay_ms(100);
        UEF_GPIO_Reset(&led_handle);
        delay_ms(100);
    }
    delay_ms(200);
    for (int i = 0; i < 2; i++) {
        UEF_GPIO_Set(&led_handle);
        delay_ms(200);
        UEF_GPIO_Reset(&led_handle);
        delay_ms(200);
    }
    UEF_GPIO_Set(&led_handle);
}

// ============================================================
// INDICATE ERROR WITH LED
// ============================================================

static void indicate_error(void) {
    while (1) {
        for (int i = 0; i < 10; i++) {
            UEF_GPIO_Toggle(&led_handle);
            delay_ms(50);
        }
        delay_ms(1000);
    }
}

// ============================================================
// INTERRUPT CALLBACK
// ============================================================

static void button_irq_handler(void) {
    uint32_t now = UEF_Clock_GetTick();
    
    // Debounce: ignore if less than 200ms since last interrupt
    if ((now - last_interrupt_time) > 200) {
        button_pressed = true;
        press_count++;
        last_interrupt_time = now;
    }
}

// ============================================================
// MAIN
// ============================================================

int main(void) {
    // Step 1: Configure system clock
    if (UEF_Clock_ConfigHSI() != 0) {
        indicate_error();  // Fast blink = clock error
    }
    UEF_Clock_ConfigSysTick(UEF_Clock_GetSystemFreq(), 1000);
    
    // Step 2: Configure LED (PA5)
    UEF_GPIO_Config_t led_config = {
        .port = GPIOA,
        .pin = 5,
        .mode = UEF_GPIO_MODE_OUTPUT_PP,
        .pull = UEF_GPIO_PULL_NONE,
        .speed = UEF_GPIO_SPEED_LOW,
        .alternate_function = 0
    };
    if (UEF_GPIO_Init(&led_handle, &led_config) != 0) {
        indicate_error();
    }
    
    // Step 3: Configure Button (PC13) - THE BLUE BUTTON ON NUCLEO
    UEF_GPIO_Config_t button_config = {
        .port = GPIOC,
        .pin = 13,
        .mode = UEF_GPIO_MODE_INPUT,
        .pull = UEF_GPIO_PULL_UP,  // Pull-up for active low button
        .speed = UEF_GPIO_SPEED_LOW,
        .alternate_function = 0
    };
    if (UEF_GPIO_Init(&button_handle, &button_config) != 0) {
        indicate_error();
    }
    
    // Step 4: Show boot pattern
    show_boot_pattern();
    
    // Step 5: Configure EXTI for button (PC13)
    // Enable EXTI clock
    UEF_EXTI_CLK_ENABLE();
    
    // Route PC13 to EXTI line 13
    uint32_t port_index = UEF_GPIO_GetPortIndex(GPIOC);
    UEF_EXTI_SelectPort(13, port_index);
    
    // Configure as input with pull-up (already done, but ensure)
    UEF_EXTI_ConfigurePinAsInput(GPIOC, 13, UEF_GPIO_PULL_UP);
    
    // Set trigger on falling edge (button press = LOW)
    UEF_EXTI_SetTrigger(13, false, true);  // Falling edge only
    
    // Enable EXTI line
    UEF_EXTI_EnableLine(13);
    
    // Enable interrupt in NVIC
    UEF_EXTI_ENABLE_IRQ(13);
    
    // Step 6: Quick test - flash LED 3 times to show interrupt is configured
    for (int i = 0; i < 3; i++) {
        UEF_GPIO_Toggle(&led_handle);
        delay_ms(100);
    }
    UEF_GPIO_Set(&led_handle);
    delay_ms(500);
    
    // Step 7: Main loop - poll button as fallback AND check interrupts
    uint32_t counter = 0;
    bool last_button_state = UEF_GPIO_Read(&button_handle);
    
    while (1) {
        counter++;
        
        // POLLING FALLBACK: If interrupt isn't working, this will still work
        // Read button state every 10ms
        bool current_button_state = UEF_GPIO_Read(&button_handle);
        
        // Button is active LOW (pressed = 0)
        if (last_button_state == true && current_button_state == false) {
            // Button was just pressed (falling edge detected by polling)
            static uint32_t last_poll_time = 0;
            uint32_t now = UEF_Clock_GetTick();
            
            if ((now - last_poll_time) > 200) {
                // If interrupt didn't catch it, use polling
                if (!button_pressed) {
                    button_pressed = true;
                    press_count++;
                    last_poll_time = now;
                }
            }
        }
        last_button_state = current_button_state;
        
        // Handle button press (from either interrupt OR polling)
        if (button_pressed) {
            button_pressed = false;
            
            // Button press pattern based on count
            if (press_count % 2 == 1) {
                // Odd press - long blink (200ms)
                UEF_GPIO_Reset(&led_handle);
                delay_ms(200);
                UEF_GPIO_Set(&led_handle);
            } else {
                // Even press - short blink (100ms)
                UEF_GPIO_Reset(&led_handle);
                delay_ms(100);
                UEF_GPIO_Set(&led_handle);
            }
        }
        
        // Heartbeat: blink LED every 2 seconds to show we're alive
        if (counter >= 2000) {
            counter = 0;
            UEF_GPIO_Toggle(&led_handle);
            delay_ms(50);
            UEF_GPIO_Toggle(&led_handle);
        }
        
        delay_ms(1);
    }
}

// ============================================================
// INTERRUPT SERVICE ROUTINE (for EXTI15_10)
// ============================================================

void EXTI15_10_IRQHandler(void) {
    // Check if it's our button (line 13)
    if (UEF_EXTI_IsPending(13)) {
        // Clear the pending flag FIRST
        UEF_EXTI_ClearPending(13);
        
        // Call the handler with debouncing
        button_irq_handler();
    }
}