#ifndef __STREAM_RING_BUFFER_H_INCLUDED_
#define __STREAM_RING_BUFFER_H_INCLUDED_

#include <stdint.h>

// types for size and offset vars extracted to customize for target buffer size.
typedef uint16_t rb_offset_t;
typedef uint16_t rb_size_t;

struct stream_rb_data_view {
    const uint8_t *data;
    rb_size_t length;
};

// ring buffer struct exposed for static allocation.
// do not access these elements directly.
struct stream_ring_buffer {
    uint8_t * const buffer;
    const rb_size_t buffer_size;
    volatile rb_size_t used;
    volatile rb_offset_t head_offset, tail_offset;
};

int stream_rb_init(struct stream_ring_buffer *rb, const uint8_t  *buffer, rb_size_t buffer_size);
int stream_rb_add(struct stream_ring_buffer *rb, const uint8_t *in, rb_size_t in_size);
int stream_rb_tail_data_view(struct stream_ring_buffer *rb, struct stream_rb_data_view *out_node_view);
int stream_rb_remove_tail(struct stream_ring_buffer *rb);


#endif //__STREAM_RING_BUFFER_H_INCLUDED_
