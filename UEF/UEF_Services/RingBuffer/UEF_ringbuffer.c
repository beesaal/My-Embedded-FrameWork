// ============================================================
// File: UEF/UEF_Services/RingBuffer/UEF_ringbuffer.c
// ============================================================
// PURPOSE: Universal Ring Buffer Service Implementation
//          Uses ONLY ChipLink.h - 100% portable
// ============================================================

#include "UEF_ringbuffer.h"
#include "ChipLink.h"  // For UEF_EnterCritical/UEF_ExitCritical
#include <string.h>

// ============================================================
// PRIVATE HELPERS
// ============================================================

static inline size_t UEF_RingBuffer_NextIndex(UEF_RingBuffer_t* rb, size_t index) {
    (void)rb;  // Suppress unused parameter warning
    return (index + 1) % rb->size;
}

// ============================================================
// INITIALIZATION
// ============================================================

int UEF_RingBuffer_Init(UEF_RingBuffer_t* rb, uint8_t* buffer, size_t size) {
    if (!rb || !buffer || size == 0) return -1;
    
    rb->buffer = buffer;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    rb->is_initialized = true;
    
    // Clear buffer
    memset(buffer, 0, size);
    
    return 0;
}

void UEF_RingBuffer_Reset(UEF_RingBuffer_t* rb) {
    if (!rb || !rb->is_initialized) return;
    
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    
    // Clear buffer contents
    memset(rb->buffer, 0, rb->size);
}

// ============================================================
// STATUS
// ============================================================

bool UEF_RingBuffer_IsEmpty(const UEF_RingBuffer_t* rb) {
    if (!rb || !rb->is_initialized) return true;
    return rb->count == 0;
}

bool UEF_RingBuffer_IsFull(const UEF_RingBuffer_t* rb) {
    if (!rb || !rb->is_initialized) return true;
    return rb->count >= rb->size;
}

size_t UEF_RingBuffer_GetSize(const UEF_RingBuffer_t* rb) {
    if (!rb || !rb->is_initialized) return 0;
    return rb->size;
}

size_t UEF_RingBuffer_GetCount(const UEF_RingBuffer_t* rb) {
    if (!rb || !rb->is_initialized) return 0;
    return rb->count;
}

size_t UEF_RingBuffer_GetFreeSpace(const UEF_RingBuffer_t* rb) {
    if (!rb || !rb->is_initialized) return 0;
    return rb->size - rb->count;
}

// ============================================================
// CRITICAL SECTION HELPERS
// ============================================================
// These should be implemented in ChipLink.h as:
//   static inline void UEF_EnterCritical(void) { __disable_irq(); }
//   static inline void UEF_ExitCritical(void) { __enable_irq(); }
// For now, we use the ChipLink macros if available, or fallback to compiler intrinsics

#ifndef UEF_ENTER_CRITICAL
    #define UEF_ENTER_CRITICAL()    __asm volatile("cpsid i" : : : "memory")
    #define UEF_EXIT_CRITICAL()     __asm volatile("cpsie i" : : : "memory")
#endif

// ============================================================
// PUSH OPERATIONS (Non-interrupt versions)
// ============================================================

int UEF_RingBuffer_Push(UEF_RingBuffer_t* rb, uint8_t data) {
    if (!rb || !rb->is_initialized) return -1;
    if (UEF_RingBuffer_IsFull(rb)) return -2;  // Buffer full
    
    rb->buffer[rb->head] = data;
    rb->head = UEF_RingBuffer_NextIndex(rb, rb->head);
    rb->count++;
    
    return 0;
}

int UEF_RingBuffer_Push_IT(UEF_RingBuffer_t* rb, uint8_t data) {
    if (!rb || !rb->is_initialized) return -1;
    
    // Disable interrupts for critical section
    UEF_ENTER_CRITICAL();
    
    int result = UEF_RingBuffer_Push(rb, data);
    
    // Re-enable interrupts
    UEF_EXIT_CRITICAL();
    
    return result;
}

int UEF_RingBuffer_PushArray(UEF_RingBuffer_t* rb, const uint8_t* data, size_t count) {
    if (!rb || !rb->is_initialized || !data || count == 0) return -1;
    
    size_t free_space = UEF_RingBuffer_GetFreeSpace(rb);
    if (count > free_space) return -2;  // Not enough space
    
    for (size_t i = 0; i < count; i++) {
        rb->buffer[rb->head] = data[i];
        rb->head = UEF_RingBuffer_NextIndex(rb, rb->head);
        rb->count++;
    }
    
    return 0;
}

int UEF_RingBuffer_PushArray_IT(UEF_RingBuffer_t* rb, const uint8_t* data, size_t count) {
    if (!rb || !rb->is_initialized) return -1;
    
    UEF_ENTER_CRITICAL();
    int result = UEF_RingBuffer_PushArray(rb, data, count);
    UEF_EXIT_CRITICAL();
    
    return result;
}

// ============================================================
// POP OPERATIONS
// ============================================================

int UEF_RingBuffer_Pop(UEF_RingBuffer_t* rb, uint8_t* data) {
    if (!rb || !rb->is_initialized || !data) return -1;
    if (UEF_RingBuffer_IsEmpty(rb)) return -2;  // Buffer empty
    
    *data = rb->buffer[rb->tail];
    rb->tail = UEF_RingBuffer_NextIndex(rb, rb->tail);
    rb->count--;
    
    return 0;
}

int UEF_RingBuffer_Pop_IT(UEF_RingBuffer_t* rb, uint8_t* data) {
    if (!rb || !rb->is_initialized) return -1;
    
    UEF_ENTER_CRITICAL();
    int result = UEF_RingBuffer_Pop(rb, data);
    UEF_EXIT_CRITICAL();
    
    return result;
}

int UEF_RingBuffer_PopArray(UEF_RingBuffer_t* rb, uint8_t* buffer, size_t count) {
    if (!rb || !rb->is_initialized || !buffer || count == 0) return -1;
    
    size_t available = UEF_RingBuffer_GetCount(rb);
    if (count > available) return -2;  // Not enough data
    
    for (size_t i = 0; i < count; i++) {
        buffer[i] = rb->buffer[rb->tail];
        rb->tail = UEF_RingBuffer_NextIndex(rb, rb->tail);
        rb->count--;
    }
    
    return 0;
}

int UEF_RingBuffer_PopArray_IT(UEF_RingBuffer_t* rb, uint8_t* buffer, size_t count) {
    if (!rb || !rb->is_initialized) return -1;
    
    UEF_ENTER_CRITICAL();
    int result = UEF_RingBuffer_PopArray(rb, buffer, count);
    UEF_EXIT_CRITICAL();
    
    return result;
}

// ============================================================
// PEEK OPERATIONS
// ============================================================

int UEF_RingBuffer_Peek(const UEF_RingBuffer_t* rb, size_t index, uint8_t* data) {
    if (!rb || !rb->is_initialized || !data) return -1;
    if (index >= rb->count) return -2;  // Index out of range
    
    size_t pos = (rb->tail + index) % rb->size;
    *data = rb->buffer[pos];
    
    return 0;
}

int UEF_RingBuffer_PeekNext(const UEF_RingBuffer_t* rb, uint8_t* data) {
    if (!rb || !rb->is_initialized || !data) return -1;
    if (UEF_RingBuffer_IsEmpty(rb)) return -2;
    
    *data = rb->buffer[rb->tail];
    return 0;
}

// ============================================================
// UTILITIES
// ============================================================

int UEF_RingBuffer_Skip(UEF_RingBuffer_t* rb, size_t count) {
    if (!rb || !rb->is_initialized) return -1;
    
    size_t available = UEF_RingBuffer_GetCount(rb);
    if (count > available) return -2;  // Not enough data
    
    for (size_t i = 0; i < count; i++) {
        rb->tail = UEF_RingBuffer_NextIndex(rb, rb->tail);
        rb->count--;
    }
    
    return 0;
}

int UEF_RingBuffer_Skip_IT(UEF_RingBuffer_t* rb, size_t count) {
    if (!rb || !rb->is_initialized) return -1;
    
    UEF_ENTER_CRITICAL();
    int result = UEF_RingBuffer_Skip(rb, count);
    UEF_EXIT_CRITICAL();
    
    return result;
}

void UEF_RingBuffer_Clear(UEF_RingBuffer_t* rb) {
    if (!rb || !rb->is_initialized) return;
    
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}