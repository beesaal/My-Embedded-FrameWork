// ============================================================
// File: UEF/UEF_Services/RingBuffer/UEF_ringbuffer.h
// ============================================================
// PURPOSE: Universal Ring Buffer Service

#ifndef UEF_RINGBUFFER_H
#define UEF_RINGBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// Ring Buffer Types
// ============================================================

typedef struct {
    uint8_t* buffer;           // Buffer memory
    size_t size;               // Buffer size in bytes
    volatile size_t head;      // Write index
    volatile size_t tail;      // Read index
    volatile size_t count;     // Number of bytes in buffer
    bool is_initialized;
} UEF_RingBuffer_t;

// ============================================================
// Ring Buffer API
// ============================================================

// Initialization
int UEF_RingBuffer_Init(UEF_RingBuffer_t* rb, uint8_t* buffer, size_t size);
void UEF_RingBuffer_Reset(UEF_RingBuffer_t* rb);

// Status
bool UEF_RingBuffer_IsEmpty(const UEF_RingBuffer_t* rb);
bool UEF_RingBuffer_IsFull(const UEF_RingBuffer_t* rb);
size_t UEF_RingBuffer_GetSize(const UEF_RingBuffer_t* rb);
size_t UEF_RingBuffer_GetCount(const UEF_RingBuffer_t* rb);
size_t UEF_RingBuffer_GetFreeSpace(const UEF_RingBuffer_t* rb);

// Push operations (Thread-safe with interrupts)
int UEF_RingBuffer_Push(UEF_RingBuffer_t* rb, uint8_t data);
int UEF_RingBuffer_Push_IT(UEF_RingBuffer_t* rb, uint8_t data);
int UEF_RingBuffer_PushArray(UEF_RingBuffer_t* rb, const uint8_t* data, size_t count);
int UEF_RingBuffer_PushArray_IT(UEF_RingBuffer_t* rb, const uint8_t* data, size_t count);

// Pop operations (Thread-safe with interrupts)
int UEF_RingBuffer_Pop(UEF_RingBuffer_t* rb, uint8_t* data);
int UEF_RingBuffer_Pop_IT(UEF_RingBuffer_t* rb, uint8_t* data);
int UEF_RingBuffer_PopArray(UEF_RingBuffer_t* rb, uint8_t* buffer, size_t count);
int UEF_RingBuffer_PopArray_IT(UEF_RingBuffer_t* rb, uint8_t* buffer, size_t count);

// Peek operations (Read without removing)
int UEF_RingBuffer_Peek(const UEF_RingBuffer_t* rb, size_t index, uint8_t* data);
int UEF_RingBuffer_PeekNext(const UEF_RingBuffer_t* rb, uint8_t* data);

// Utilities
int UEF_RingBuffer_Skip(UEF_RingBuffer_t* rb, size_t count);
int UEF_RingBuffer_Skip_IT(UEF_RingBuffer_t* rb, size_t count);
void UEF_RingBuffer_Clear(UEF_RingBuffer_t* rb);

#ifdef __cplusplus
}
#endif

#endif // UEF_RINGBUFFER_H