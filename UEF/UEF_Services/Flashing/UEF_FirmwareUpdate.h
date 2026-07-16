#ifndef UEF_FIRMWARE_UPDATE_H
#define UEF_FIRMWARE_UPDATE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the update system
void UEF_FirmwareUpdate_Init(uint8_t* buffer, uint32_t buffer_size);

// Set UART handle for sending ACK/NAK
void UEF_FirmwareUpdate_SetUartHandle(UEF_UART_Handle_t* handle);

// Start a firmware update
void UEF_FirmwareUpdate_Start(uint32_t total_size);

// Process incoming data (call from UART callback)
void UEF_FirmwareUpdate_ProcessData(const uint8_t* data, uint32_t length);

// Get current progress (0-100)
uint32_t UEF_FirmwareUpdate_GetProgress(void);

// Register callbacks
void UEF_FirmwareUpdate_RegisterCallbacks(
    void (*on_progress)(uint32_t percent),
    void (*on_complete)(int success));

// Get error code
uint32_t UEF_FirmwareUpdate_GetError(void);

#ifdef __cplusplus
}
#endif

#endif