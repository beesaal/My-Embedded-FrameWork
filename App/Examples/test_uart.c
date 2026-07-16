// ============================================================
// File: App/Examples/test_uart.c
// PURPOSE: Comprehensive UART driver test
//          Tests: Send, Receive, Interrupts, Callbacks, Echo
// ============================================================

#include "UEF_clock.h"
#include "UEF_uart.h"
#include "UEF_gpio.h"
#include "UEF_logger.h"
#include "UEF_ringbuffer.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// ============================================================
// HANDLES
// ============================================================

static UEF_UART_Handle_t uart_handle;
static UEF_GPIO_Handle_t led_handle;
static UEF_Logger_t logger;
static UEF_RingBuffer_t rx_ringbuf;
static uint8_t rx_buffer[256];

// ============================================================
// TEST STATE
// ============================================================

static volatile uint32_t tx_complete_count = 0;
static volatile uint32_t rx_complete_count = 0;
static volatile uint32_t rx_byte_count = 0;
static volatile uint32_t error_count = 0;
static volatile bool echo_mode = true;

// ============================================================
// LOGGER OUTPUT
// ============================================================

static void logger_output_callback(const char* str, uint16_t len) {
    (void)len;
    if (uart_handle.is_initialized) {
        UEF_UART_SendString(&uart_handle, str);
    }
}

// ============================================================
// DELAY
// ============================================================

static void delay_ms(uint32_t ms) {
    UEF_DelayMs(ms);
}

// ============================================================
// UART CALLBACKS
// ============================================================

static void uart_tx_complete(UEF_UART_Handle_t* handle) {
    (void)handle;
    tx_complete_count++;
}

static void uart_rx_complete(UEF_UART_Handle_t* handle) {
    (void)handle;
    rx_complete_count++;
}

static void uart_error_callback(UEF_UART_Handle_t* handle, uint32_t error) {
    (void)handle;
    error_count++;
    UEF_LOGE(&logger, "UART Error: 0x%08lX", (unsigned long)error);
}

static void uart_rx_byte_callback(UEF_UART_Handle_t* handle, uint8_t byte) {
    (void)handle;
    rx_byte_count++;
    
    // Store in ring buffer
    UEF_RingBuffer_Push_IT(&rx_ringbuf, byte);
    
    // Echo mode
    if (echo_mode) {
        UEF_UART_SendByte(&uart_handle, byte);
        
        // Special commands
        if (byte == '1') {
            UEF_GPIO_Set(&led_handle);
            UEF_UART_SendString(&uart_handle, "\r\nLED ON\r\n");
        } else if (byte == '0') {
            UEF_GPIO_Reset(&led_handle);
            UEF_UART_SendString(&uart_handle, "\r\nLED OFF\r\n");
        } else if (byte == 'e') {
            echo_mode = !echo_mode;
            UEF_UART_SendString(&uart_handle, echo_mode ? "\r\nEcho ON\r\n" : "\r\nEcho OFF\r\n");
        } else if (byte == 's') {
            char msg[64];
            sprintf(msg, "\r\nStats: TX=%lu, RX=%lu, Errors=%lu, Echo=%s\r\n",
                    (unsigned long)tx_complete_count,
                    (unsigned long)rx_byte_count,
                    (unsigned long)error_count,
                    echo_mode ? "ON" : "OFF");
            UEF_UART_SendString(&uart_handle, msg);
        } else if (byte == '?' || byte == 'h') {
            UEF_UART_SendString(&uart_handle, "\r\nCommands:\r\n");
            UEF_UART_SendString(&uart_handle, "  '0' - LED OFF\r\n");
            UEF_UART_SendString(&uart_handle, "  '1' - LED ON\r\n");
            UEF_UART_SendString(&uart_handle, "  'e' - Toggle echo\r\n");
            UEF_UART_SendString(&uart_handle, "  's' - Show stats\r\n");
            UEF_UART_SendString(&uart_handle, "  '?' - Show help\r\n");
            UEF_UART_SendString(&uart_handle, "  'r' - Reset MCU\r\n");
            UEF_UART_SendString(&uart_handle, "\r\n");
        } else if (byte == 'r' || byte == 'R') {
            UEF_UART_SendString(&uart_handle, "\r\nResetting MCU...\r\n");
            delay_ms(100);
            NVIC_SystemReset();
        }
    }
}

// ============================================================
// TEST: SEND STRINGS
// ============================================================

static void test_send_strings(void) {
    UEF_LOGI(&logger, "=== Testing String Transmission ===");
    
    const char* test_strings[] = {
        "Hello, World!",
        "This is a test message.",
        "UART communication is working!",
        "1234567890",
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
        "abcdefghijklmnopqrstuvwxyz",
        "Special chars: !@#$%^&*()",
        "Line 1\r\nLine 2\r\nLine 3",
        "Long message: ",
        "The quick brown fox jumps over the lazy dog."
    };
    
    for (int i = 0; i < 10; i++) {
        UEF_UART_SendString(&uart_handle, test_strings[i]);
        UEF_UART_SendString(&uart_handle, "\r\n");
        delay_ms(100);
    }
    
    UEF_LOGI(&logger, "String transmission test complete");
}

// ============================================================
// TEST: SEND BYTES
// ============================================================

static void test_send_bytes(void) {
    UEF_LOGI(&logger, "=== Testing Byte Transmission ===");
    
    // Send incrementing values
    UEF_UART_SendString(&uart_handle, "Incrementing bytes: ");
    for (int i = 0; i < 32; i++) {
        UEF_UART_SendByte(&uart_handle, (uint8_t)i);
    }
    UEF_UART_SendString(&uart_handle, "\r\n");
    
    // Send hex pattern
    UEF_UART_SendString(&uart_handle, "Hex pattern: ");
    uint8_t hex_pattern[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x12, 0x34, 0x56, 0x78};
    UEF_UART_SendBytes(&uart_handle, hex_pattern, sizeof(hex_pattern));
    UEF_UART_SendString(&uart_handle, "\r\n");
    
    // Send ASCII table
    UEF_UART_SendString(&uart_handle, "ASCII table (32-126): ");
    for (int i = 32; i < 127; i++) {
        UEF_UART_SendByte(&uart_handle, (uint8_t)i);
    }
    UEF_UART_SendString(&uart_handle, "\r\n");
    
    UEF_LOGI(&logger, "Byte transmission test complete");
}

// ============================================================
// TEST: BAUD RATE CHANGES
// ============================================================

static void test_baud_rate(void) {
    UEF_LOGI(&logger, "=== Testing Baud Rate Changes ===");
    UEF_LOGI(&logger, "Changing to 9600 baud...");
    
    UEF_UART_SendString(&uart_handle, "This is at 115200 baud\r\n");
    delay_ms(100);
    
    // Change to 9600
    UEF_UART_SetBaudRate(&uart_handle, 9600);
    delay_ms(100);
    UEF_UART_SendString(&uart_handle, "This is at 9600 baud\r\n");
    delay_ms(100);
    
    // Change back to 115200
    UEF_LOGI(&logger, "Changing back to 115200 baud...");
    UEF_UART_SetBaudRate(&uart_handle, 115200);
    delay_ms(100);
    UEF_UART_SendString(&uart_handle, "Back to 115200 baud\r\n");
    
    UEF_LOGI(&logger, "Baud rate test complete");
}

// ============================================================
// TEST: INTERRUPT MODE
// ============================================================

static void test_interrupt_mode(void) {
    UEF_LOGI(&logger, "=== Testing Interrupt Mode ===");
    UEF_LOGI(&logger, "Interrupts enabled - type characters to see echo");
    UEF_LOGI(&logger, "Stats: TX=%lu, RX=%lu", 
             (unsigned long)tx_complete_count,
             (unsigned long)rx_byte_count);
    
    // Send test data with interrupts
    UEF_UART_SendByte_IT(&uart_handle, 'A');
    delay_ms(100);
    UEF_UART_SendByte_IT(&uart_handle, 'B');
    delay_ms(100);
    UEF_UART_SendByte_IT(&uart_handle, 'C');
    delay_ms(100);
    
    UEF_LOGI(&logger, "Interrupt test complete");
    UEF_LOGI(&logger, "Stats: TX=%lu, RX=%lu", 
             (unsigned long)tx_complete_count,
             (unsigned long)rx_byte_count);
}

// ============================================================
// TEST: RECEIVE BUFFER
// ============================================================

static void test_receive_buffer(void) {
    UEF_LOGI(&logger, "=== Testing Receive Buffer ===");
    
    // Read all pending bytes from ring buffer
    uint8_t buffer[64];
    size_t count = UEF_RingBuffer_GetCount(&rx_ringbuf);
    
    if (count > 0) {
        UEF_LOGI(&logger, "Reading %lu bytes from ring buffer", (unsigned long)count);
        UEF_RingBuffer_PopArray(&rx_ringbuf, buffer, count);
        
        UEF_UART_SendString(&uart_handle, "Received: ");
        UEF_UART_SendBytes(&uart_handle, buffer, count);
        UEF_UART_SendString(&uart_handle, "\r\n");
    } else {
        UEF_LOGI(&logger, "No data in ring buffer");
    }
    
    UEF_LOGI(&logger, "Receive buffer test complete");
}

// ============================================================
// TEST: UART STATUS
// ============================================================

static void test_status(void) {
    UEF_LOGI(&logger, "=== Testing UART Status ===");
    
    bool tx_ready = UEF_UART_IsTxReady(&uart_handle);
    bool rx_ready = UEF_UART_IsRxReady(&uart_handle);
    bool tx_complete = UEF_UART_IsTxComplete(&uart_handle);
    uint32_t errors = UEF_UART_GetErrorFlags(&uart_handle);
    
    UEF_LOGI(&logger, "TX Ready: %s", tx_ready ? "Yes" : "No");
    UEF_LOGI(&logger, "RX Ready: %s", rx_ready ? "Yes" : "No");
    UEF_LOGI(&logger, "TX Complete: %s", tx_complete ? "Yes" : "No");
    UEF_LOGI(&logger, "Errors: 0x%08lX", (unsigned long)errors);
    
    UEF_LOGI(&logger, "Stats:");
    UEF_LOGI(&logger, "  TX Complete Callbacks: %lu", (unsigned long)tx_complete_count);
    UEF_LOGI(&logger, "  RX Complete Callbacks: %lu", (unsigned long)rx_complete_count);
    UEF_LOGI(&logger, "  RX Bytes Received: %lu", (unsigned long)rx_byte_count);
    UEF_LOGI(&logger, "  Errors: %lu", (unsigned long)error_count);
    
    UEF_LOGI(&logger, "Status test complete");
}

// ============================================================
// MAIN
// ============================================================

int main(void) {
    // Step 1: Configure system clock
    UEF_Clock_ConfigHSI();
    UEF_Clock_ConfigSysTick(UEF_Clock_GetSystemFreq(), 1000);
    
    // Step 2: Initialize LED
    UEF_GPIO_Config_t led_config = {
        .port = GPIOA,
        .pin = 5,
        .mode = UEF_GPIO_MODE_OUTPUT_PP,
        .pull = UEF_GPIO_PULL_NONE,
        .speed = UEF_GPIO_SPEED_LOW,
        .alternate_function = 0
    };
    UEF_GPIO_Init(&led_handle, &led_config);
    
    // Step 3: Initialize Ring Buffer
    UEF_RingBuffer_Init(&rx_ringbuf, rx_buffer, sizeof(rx_buffer));
    
    // Step 4: Initialize Logger
    UEF_Logger_Init(&logger);
    UEF_Logger_SetOutputCallback(&logger, logger_output_callback);
    UEF_Logger_SetLevel(&logger, UEF_LOG_LEVEL_INFO);
    
    // Step 5: Initialize UART (with interrupts)
    UEF_UART_Config_t config = {
        .instance = 2,
        .baudrate = 115200,
        .data_bits = UEF_UART_DATA_8,
        .stop_bits = UEF_UART_STOP_1,
        .parity = UEF_UART_PARITY_NONE,
        .enable_tx = true,
        .enable_rx = true,
        .enable_interrupts = true
    };
    
    if (UEF_UART_Init(&uart_handle, &config) != 0) {
        // UART failed - fast blink forever
        while (1) {
            UEF_GPIO_Set(&led_handle);
            delay_ms(50);
            UEF_GPIO_Reset(&led_handle);
            delay_ms(50);
        }
    }
    
    // Step 6: Register callbacks
    UEF_UART_RegisterCallbacks(&uart_handle, 
                               uart_tx_complete,
                               uart_rx_complete,
                               uart_error_callback);
    uart_handle.rx_byte_cb = uart_rx_byte_callback;
    
    // Step 7: Boot LED pattern
    for (int i = 0; i < 3; i++) {
        UEF_GPIO_Set(&led_handle);
        delay_ms(100);
        UEF_GPIO_Reset(&led_handle);
        delay_ms(100);
    }
    UEF_GPIO_Set(&led_handle);
    delay_ms(500);
    
    // Step 8: Print header
    UEF_UART_SendString(&uart_handle, "\r\n");
    UEF_UART_SendString(&uart_handle, "========================================\r\n");
    UEF_UART_SendString(&uart_handle, "UEF UART DRIVER TEST v1.0\r\n");
    UEF_UART_SendString(&uart_handle, "========================================\r\n");
    UEF_UART_SendString(&uart_handle, "Baud Rate: 115200, 8N1\r\n");
    UEF_UART_SendString(&uart_handle, "MCU: STM32F411RE\r\n");
    UEF_UART_SendString(&uart_handle, "========================================\r\n\r\n");
    
    // Step 9: Run tests
    test_send_strings();
    delay_ms(500);
    
    test_send_bytes();
    delay_ms(500);
    
    test_baud_rate();
    delay_ms(500);
    
    test_interrupt_mode();
    delay_ms(500);
    
    test_status();
    delay_ms(500);
    
    test_receive_buffer();
    delay_ms(500);
    
    // Step 10: Final message
    UEF_UART_SendString(&uart_handle, "\r\n");
    UEF_UART_SendString(&uart_handle, "========================================\r\n");
    UEF_UART_SendString(&uart_handle, "✅ ALL UART TESTS COMPLETED!\r\n");
    UEF_UART_SendString(&uart_handle, "========================================\r\n");
    UEF_UART_SendString(&uart_handle, "Type something and it will echo back!\r\n");
    UEF_UART_SendString(&uart_handle, "Commands: 0=LED OFF, 1=LED ON, e=Echo, s=Stats, ?=Help\r\n");
    UEF_UART_SendString(&uart_handle, "========================================\r\n\r\n");
    
    // Step 11: Main loop
    uint32_t counter = 0;
    while (1) {
        counter++;
        if (counter >= 1000) {
            counter = 0;
            UEF_GPIO_Toggle(&led_handle);
        }
        delay_ms(1);
    }
}

// ============================================================
// SYSCALL OVERRIDE - Redirect printf to UART
// ============================================================

int _write(int file, char *ptr, int len) {
    (void)file;
    if (uart_handle.is_initialized && len > 0) {
        UEF_UART_SendBytes(&uart_handle, (const uint8_t*)ptr, len);
    }
    return len;
}