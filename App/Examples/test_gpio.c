/*
OUTPUT: Boot Sequence (Right after programming)
        First pattern: blink_led(3, 100, 100)
            LED blinks 3 times fast (100ms ON, 100ms OFF)
            Pattern: ON-OFF-ON-OFF-ON-OFF
            Pause: 200ms

        Second pattern: blink_led(2, 200, 200)
            LED blinks 2 times slower (200ms ON, 200ms OFF)
            Pattern: ON-OFF-ON-OFF
            LED stays ON (after boot)

*/
// ============================================================
// File: App/Examples/test_gpio.c
// PURPOSE: Comprehensive GPIO driver test
//          Tests: LED, Button, GPIO modes, Port operations
//          NO UART REQUIRED - uses only GPIO + Clock
// ============================================================

#include "UEF_clock.h"
#include "UEF_gpio.h"
#include <stdio.h>
#include <string.h>

// ============================================================
// HANDLES
// ============================================================

static UEF_GPIO_Handle_t led_handle;
static UEF_GPIO_Handle_t button_handle;
static UEF_GPIO_Handle_t test_output_handle;
static UEF_GPIO_Handle_t test_input_handle;

// ============================================================
// DELAY
// ============================================================

static void delay_ms(uint32_t ms) {
    UEF_DelayMs(ms);
}

// ============================================================
// LED PATTERNS
// ============================================================

static void blink_led(uint32_t count, uint32_t on_ms, uint32_t off_ms) {
    for (uint32_t i = 0; i < count; i++) {
        UEF_GPIO_Set(&led_handle);
        delay_ms(on_ms);
        UEF_GPIO_Reset(&led_handle);
        delay_ms(off_ms);
    }
}

static void show_boot_pattern(void) {
    blink_led(3, 100, 100);
    delay_ms(200);
    blink_led(2, 200, 200);
    UEF_GPIO_Set(&led_handle);
}

// ============================================================
// TEST: LED BLINKING
// ============================================================

static void test_led_blink(void) {
    // Visual test only - no UART output
    printf("\r\n=== Testing LED Blink ===\r\n");
    blink_led(5, 100, 100);
    delay_ms(200);
    blink_led(3, 300, 300);
    delay_ms(200);
    blink_led(2, 500, 500);
    printf("LED blink test complete\r\n");
}

// ============================================================
// TEST: BUTTON POLLING
// ============================================================

static void test_button_polling(void) {
    printf("\r\n=== Testing Button Polling ===\r\n");
    printf("Press the button to see LED toggle\r\n");
    printf("Watching for 10 seconds...\r\n");
    
    bool last_state = UEF_GPIO_Read(&button_handle);
    uint32_t start_time = UEF_Clock_GetTick();
    uint32_t press_count = 0;
    
    while ((UEF_Clock_GetTick() - start_time) < 10000) {
        bool current_state = UEF_GPIO_Read(&button_handle);
        
        if (last_state == true && current_state == false) {
            press_count++;
            UEF_GPIO_Toggle(&led_handle);
            printf("Button pressed! Count: %lu\r\n", (unsigned long)press_count);
        }
        
        last_state = current_state;
        delay_ms(10);
    }
    
    printf("Button press count: %lu\r\n", (unsigned long)press_count);
}

// ============================================================
// TEST: OUTPUT MODES
// ============================================================

static void test_output_modes(void) {
    printf("\r\n=== Testing GPIO Output Modes ===\r\n");
    
    // Push-Pull
    printf("Push-Pull: Toggling...\r\n");
    UEF_GPIO_SetMode(&test_output_handle, UEF_GPIO_MODE_OUTPUT_PP);
    for (int i = 0; i < 5; i++) {
        UEF_GPIO_Set(&test_output_handle);
        delay_ms(200);
        UEF_GPIO_Reset(&test_output_handle);
        delay_ms(200);
    }
    
    // Open-Drain with pull-up
    printf("Open-Drain with pull-up...\r\n");
    UEF_GPIO_SetMode(&test_output_handle, UEF_GPIO_MODE_OUTPUT_OD);
    UEF_GPIO_SetPull(&test_output_handle, UEF_GPIO_PULL_UP);
    for (int i = 0; i < 3; i++) {
        UEF_GPIO_Set(&test_output_handle);
        delay_ms(200);
        UEF_GPIO_Reset(&test_output_handle);
        delay_ms(200);
    }
    
    printf("Output modes test complete\r\n");
}

// ============================================================
// TEST: INPUT MODES
// ============================================================

static void test_input_modes(void) {
    printf("\r\n=== Testing GPIO Input Modes ===\r\n");
    
    printf("Pull-up: Connect pin to GND to see change\r\n");
    UEF_GPIO_SetMode(&test_input_handle, UEF_GPIO_MODE_INPUT);
    UEF_GPIO_SetPull(&test_input_handle, UEF_GPIO_PULL_UP);
    for (int i = 0; i < 5; i++) {
        bool state = UEF_GPIO_Read(&test_input_handle);
        printf("  State: %s\r\n", state ? "HIGH" : "LOW");
        delay_ms(500);
    }
    
    printf("Pull-down: Connect pin to VCC to see change\r\n");
    UEF_GPIO_SetPull(&test_input_handle, UEF_GPIO_PULL_DOWN);
    for (int i = 0; i < 5; i++) {
        bool state = UEF_GPIO_Read(&test_input_handle);
        printf("  State: %s\r\n", state ? "HIGH" : "LOW");
        delay_ms(500);
    }
    
    printf("Input modes test complete\r\n");
}

// ============================================================
// TEST: PORT OPERATIONS
// ============================================================

static void test_port_operations(void) {
    printf("\r\n=== Testing Port Operations ===\r\n");
    
    uint32_t patterns[] = {0xAAAAAAAA, 0x55555555, 0xFFFFFFFF, 0x00000000};
    const char* desc[] = {"0xAAAAAAAA", "0x55555555", "0xFFFFFFFF", "0x00000000"};
    
    for (int i = 0; i < 4; i++) {
        printf("Writing %s to GPIOA...\r\n", desc[i]);
        UEF_GPIO_WritePort(GPIOA, patterns[i]);
        delay_ms(500);
        printf("Read back: 0x%08lX\r\n", (unsigned long)UEF_GPIO_ReadPort(GPIOA));
        delay_ms(500);
    }
    
    printf("Port operations test complete\r\n");
}

// ============================================================
// MAIN
// ============================================================

int main(void) {
    // Step 1: Configure system clock
    UEF_Clock_ConfigHSI();
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
    UEF_GPIO_Init(&led_handle, &led_config);
    
    // Step 3: Configure Button (PC13)
    UEF_GPIO_Config_t button_config = {
        .port = GPIOC,
        .pin = 13,
        .mode = UEF_GPIO_MODE_INPUT,
        .pull = UEF_GPIO_PULL_UP,
        .speed = UEF_GPIO_SPEED_LOW,
        .alternate_function = 0
    };
    UEF_GPIO_Init(&button_handle, &button_config);
    
    // Step 4: Configure Test Output (PA0)
    UEF_GPIO_Config_t output_config = {
        .port = GPIOA,
        .pin = 0,
        .mode = UEF_GPIO_MODE_OUTPUT_PP,
        .pull = UEF_GPIO_PULL_NONE,
        .speed = UEF_GPIO_SPEED_MEDIUM,
        .alternate_function = 0
    };
    UEF_GPIO_Init(&test_output_handle, &output_config);
    
    // Step 5: Configure Test Input (PA1)
    UEF_GPIO_Config_t input_config = {
        .port = GPIOA,
        .pin = 1,
        .mode = UEF_GPIO_MODE_INPUT,
        .pull = UEF_GPIO_PULL_NONE,
        .speed = UEF_GPIO_SPEED_LOW,
        .alternate_function = 0
    };
    UEF_GPIO_Init(&test_input_handle, &input_config);
    
    // Step 6: Boot pattern
    show_boot_pattern();
    
    // Step 7: Header
    printf("\r\n");
    printf("========================================\r\n");
    printf("UEF GPIO DRIVER TEST v1.0\r\n");
    printf("========================================\r\n");
    printf("MCU: %s\r\n", CHIPLINK_MCU);
    printf("========================================\r\n\r\n");
    
    // Step 8: Run tests
    test_led_blink();
    delay_ms(500);
    
    test_button_polling();
    delay_ms(500);
    
    test_output_modes();
    delay_ms(500);
    
    test_input_modes();
    delay_ms(500);
    
    test_port_operations();
    delay_ms(500);
    
    // Step 9: Final
    printf("\r\n");
    printf("========================================\r\n");
    printf("✅ ALL GPIO TESTS COMPLETED!\r\n");
    printf("========================================\r\n");
    printf("Press button to toggle LED\r\n");
    printf("========================================\r\n\r\n");
    
    // Step 10: Main loop
    uint32_t counter = 0;
    while (1) {
        counter++;
        if (counter >= 1000) {
            counter = 0;
            UEF_GPIO_Toggle(&led_handle);
        }
        
        if (!UEF_GPIO_Read(&button_handle)) {
            UEF_GPIO_Toggle(&led_handle);
            delay_ms(200);
        }
        
        delay_ms(1);
    }
}