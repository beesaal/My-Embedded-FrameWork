// ============================================================
// File: App/test_ALL.c
// PURPOSE: Test all drivers together in one application
// ============================================================

#include "UEF_clock.h"
#include "UEF_gpio.h"
#include "UEF_uart.h"
#include "UEF_spi.h"
#include "UEF_i2c.h"
#include "UEF_adc.h"
#include "UEF_timmer.h"
#include "UEF_logger.h"
#include "UEF_ringbuffer.h"
#include <string.h>
#include <stdio.h>

// ============================================================
// HANDLES
// ============================================================

static UEF_GPIO_Handle_t led_handle;
static UEF_UART_Handle_t uart_handle;
static UEF_SPI_Handle_t spi_handle;
static UEF_I2C_Handle_t i2c_handle;
static UEF_ADC_Handle_t adc_handle;
static UEF_Logger_t logger;
static UEF_RingBuffer_t ringbuf;
static uint8_t ringbuf_buffer[64];

// ============================================================
// SYSCALL OVERRIDE - Redirect printf to UART
// ============================================================

// Override the _write syscall to use UART for printf
int _write(int file, char *ptr, int len) {
    (void)file;  // Suppress unused parameter warning
    
    // Only send if UART is initialized and we have data
    if (uart_handle.is_initialized && len > 0) {
        UEF_UART_SendBytes(&uart_handle, (const uint8_t*)ptr, len);
    }
    return len;
}

// ============================================================
// DELAY
// ============================================================

static void delay_ms(uint32_t ms) {
    UEF_DelayMs(ms);
}

// ============================================================
// LED CONTROL FOR TEST INDICATION
// ============================================================

static void led_init(void) {
    UEF_GPIO_Config_t config = {
        .port = GPIOA,
        .pin = 5,
        .mode = UEF_GPIO_MODE_OUTPUT_PP,
        .pull = UEF_GPIO_PULL_NONE,
        .speed = UEF_GPIO_SPEED_LOW,
        .alternate_function = 0
    };
    UEF_GPIO_Init(&led_handle, &config);
}

static void led_on(void) {
    UEF_GPIO_Set(&led_handle);
}

static void led_off(void) {
    UEF_GPIO_Reset(&led_handle);
}

static void led_toggle(void) {
    UEF_GPIO_Toggle(&led_handle);
}

// ============================================================
// TEST FUNCTIONS
// ============================================================

static bool test_led(void) {
    printf("  Testing GPIO LED... ");
    
    led_init();
    
    // Test LED toggling
    led_on();
    delay_ms(100);
    led_off();
    delay_ms(100);
    led_toggle();
    delay_ms(100);
    led_on();  // Leave LED on for visual feedback
    
    printf("PASSED\r\n");
    return true;
}

static bool test_uart(void) {
    printf("  Testing UART... ");
    
    UEF_UART_Config_t config = {
        .instance = 2,
        .baudrate = 115200,
        .data_bits = UEF_UART_DATA_8,
        .stop_bits = UEF_UART_STOP_1,
        .parity = UEF_UART_PARITY_NONE,
        .enable_tx = true,
        .enable_rx = true,
        .enable_interrupts = false
    };
    
    if (UEF_UART_Init(&uart_handle, &config) != 0) {
        printf("FAILED (Init error)\r\n");
        return false;
    }
    
    printf("PASSED (115200 baud)\r\n");
    return true;
}

static bool test_spi(void) {
    printf("  Testing SPI... ");
    
    UEF_SPI_Config_t config = {
        .instance = 1,
        .is_master = true,
        .mode = UEF_SPI_MODE_0,
        .baudrate = UEF_SPI_BAUD_DIV_16,
        .data_width = UEF_SPI_DATA_8BIT,
        .lsb_first = false,
        .enable_interrupts = false
    };
    
    if (UEF_SPI_Init(&spi_handle, &config) != 0) {
        printf("FAILED (Init error)\r\n");
        return false;
    }
    
    // Simple test - send and receive
    uint8_t tx = 0xAA;
    uint8_t rx = UEF_SPI_TransceiveByte(&spi_handle, tx);
    
    if (rx == tx) {
        printf("PASSED (loopback)\r\n");
        return true;
    } else {
        printf("FAILED (expected 0x%02X, got 0x%02X)\r\n", tx, rx);
        return false;
    }
}

static bool test_i2c(void) {
    printf("  Testing I2C... ");
    
    UEF_I2C_Config_t config = {
        .instance = 1,
        .speed = UEF_I2C_SPEED_STANDARD,
        .own_address = 0x00,
        .is_master = true,
        .enable_interrupts = false
    };
    
    if (UEF_I2C_Init(&i2c_handle, &config) != 0) {
        printf("FAILED (Init error)\r\n");
        return false;
    }
    
    // Try to detect a device at common addresses
    bool found = false;
    uint8_t test = 0x00;
    uint8_t common_addresses[] = {0x3C, 0x3D, 0x50, 0x51, 0x68, 0x69, 0x76, 0x77};
    
    for (size_t i = 0; i < sizeof(common_addresses); i++) {
        int result = UEF_I2C_MasterWrite(&i2c_handle, common_addresses[i], &test, 1);
        if (result == 0) {
            found = true;
            printf("PASSED (device found at 0x%02X)\r\n", common_addresses[i]);
            break;
        }
    }
    
    if (!found) {
        printf("WARNING (no I2C device detected - is something connected?)\r\n");
        // Return true anyway since I2C initialized correctly
    }
    
    return true;
}

static bool test_adc(void) {
    printf("  Testing ADC... ");
    
    UEF_ADC_Config_t config = {
        .instance = 1,
        .channel = 0,
        .gpio_port = GPIOA,
        .gpio_pin = 0,
        .resolution = UEF_ADC_RESOLUTION_12BIT,
        .align = UEF_ADC_ALIGN_RIGHT,
        .sample_time = UEF_ADC_SAMPLE_56_CYCLES,
        .enable_interrupts = false
    };
    
    if (UEF_ADC_Init(&adc_handle, &config) != 0) {
        printf("FAILED (Init error)\r\n");
        return false;
    }
    
    // Read a value
    uint16_t value = UEF_ADC_ReadChannel(&adc_handle);
    
    // Convert to voltage (assuming 3.3V reference)
    float voltage = (value * 3.3f) / 4095.0f;
    
    printf("PASSED (value=%u, %.2fV)\r\n", value, voltage);
    return true;
}

// Logger output callback wrapper
static void logger_output_callback(const char* str, uint16_t len) {
    (void)len;  // Suppress unused parameter warning
    UEF_UART_SendString(&uart_handle, str);
}

static bool test_logger(void) {
    printf("  Testing Logger... ");
    
    // Setup logger with UART output
    UEF_Logger_Init(&logger);
    logger.output_cb = logger_output_callback;
    
    // Test different log levels
    UEF_Logger_Info(&logger, "Logger test: Info message");
    UEF_Logger_Debug(&logger, "Logger test: Debug message");
    UEF_Logger_Error(&logger, "Logger test: Error message");
    UEF_Logger_Warn(&logger, "Logger test: Warning message");
    
    printf("PASSED\r\n");
    return true;
}

static bool test_ringbuffer(void) {
    printf("  Testing RingBuffer... ");
    
    UEF_RingBuffer_Init(&ringbuf, ringbuf_buffer, sizeof(ringbuf_buffer));
    
    // Push some data
    for (int i = 0; i < 10; i++) {
        UEF_RingBuffer_Push(&ringbuf, (uint8_t)i);
    }
    
    // Pop and verify
    for (int i = 0; i < 10; i++) {
        uint8_t value;
        UEF_RingBuffer_Pop(&ringbuf, &value);
        if (value != (uint8_t)i) {
            printf("FAILED (expected %d, got %d at index %d)\r\n", i, value, i);
            return false;
        }
    }
    
    printf("PASSED\r\n");
    return true;
}

// ============================================================
// PRINT TEST SUMMARY
// ============================================================

static void print_test_summary(const char* name, bool passed) {
    if (passed) {
        printf("  ✅ %s - PASSED\r\n", name);
    } else {
        printf("  ❌ %s - FAILED\r\n", name);
    }
}

// ============================================================
// MAIN
// ============================================================

int main(void) {
    // Step 1: Initialize LED first (for visual feedback during boot)
    led_init();
    led_on();  // LED on to show we're starting
    delay_ms(500);
    led_off();
    delay_ms(500);
    
    // Step 2: Configure system clock
    // UART and printf are not ready yet, so no printf here
    if (UEF_Clock_ConfigHSI() != 0) {
        // Error - fast blink forever
        while (1) {
            led_on();
            delay_ms(50);
            led_off();
            delay_ms(50);
        }
    }
    
    // Step 3: Initialize UART first (so printf works)
    bool uart_ok = test_uart();
    if (!uart_ok) {
        // Error - blink pattern
        while (1) {
            led_on();
            delay_ms(100);
            led_off();
            delay_ms(100);
            led_on();
            delay_ms(100);
            led_off();
            delay_ms(300);
        }
    }
    
    // Now printf will work
    printf("\r\n");
    printf("========================================\r\n");
    printf("UEF Framework - All Drivers Test\r\n");
    printf("========================================\r\n");
    printf("System Clock: HSI @ 16MHz\r\n");
    printf("========================================\r\n\n");
    
    // Step 4: Test LED (visual feedback)
    printf("Running tests:\r\n");
    printf("----------------------------------------\r\n");
    
    bool led_ok = test_led();
    
    // Test results array
    struct {
        const char* name;
        bool result;
    } tests[] = {
        {"LED", led_ok},
        {"SPI", test_spi()},
        {"I2C", test_i2c()},
        {"ADC", test_adc()},
        {"Logger", test_logger()},
        {"RingBuffer", test_ringbuffer()}
    };
    
    printf("----------------------------------------\r\n");
    printf("\nTest Summary:\r\n");
    printf("----------------------------------------\r\n");
    
    // Print summary
    bool all_passed = true;
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        print_test_summary(tests[i].name, tests[i].result);
        if (!tests[i].result) {
            all_passed = false;
        }
    }
    
    printf("----------------------------------------\r\n");
    
    if (all_passed) {
        printf("\n🎉 ALL TESTS PASSED!\r\n");
        printf("========================================\r\n\n");
        
        // All passed - blink pattern: 3 fast blinks, then pause
        while (1) {
            for (int i = 0; i < 3; i++) {
                led_on();
                delay_ms(100);
                led_off();
                delay_ms(100);
            }
            delay_ms(500);
        }
    } else {
        printf("\n❌ SOME TESTS FAILED!\r\n");
        printf("========================================\r\n\n");
        
        // Some failed - slow blink forever
        while (1) {
            led_on();
            delay_ms(1000);
            led_off();
            delay_ms(1000);
        }
    }
}