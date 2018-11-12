#ifndef __STREAM_RING_BUFFER_H_INCLUDED_
#define __STREAM_RING_BUFFER_H_INCLUDED_

#include <stdint.h>

// types for size and offset vars extracted to customize for target buffer size.
// be sure to size stream_rb_size_t such that 
// it can count up to N without overflow for a buffer of size N
typedef uint16_t stream_rb_offset_t;
typedef uint16_t stream_rb_size_t;

struct stream_rb_data_view {
    const uint8_t *data;
    const stream_rb_size_t length;
};

// ring buffer struct exposed for static allocation.
// do not access these elements directly.
struct stream_ring_buffer {
    uint8_t * const buffer;
    const stream_rb_size_t buffer_size;
    volatile stream_rb_size_t used;
    volatile stream_rb_offset_t head_offset, tail_offset;
};

int stream_rb_init(struct stream_ring_buffer *rb, const uint8_t  *buffer, stream_rb_size_t buffer_size);
int stream_rb_add(struct stream_ring_buffer *rb, const uint8_t *in, stream_rb_size_t in_size);
int stream_rb_tail_data_view(struct stream_ring_buffer *rb, struct stream_rb_data_view *out_node_view);
int stream_rb_remove_tail(struct stream_ring_buffer *rb);

#ifdef STREAM_RING_BUFFER_DEBUG
void stream_rb_print_state(struct stream_ring_buffer *rb);
#endif

#endif //__STREAM_RING_BUFFER_H_INCLUDED_
