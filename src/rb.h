#ifndef __RB_DMA_H_INCLUDED_
#define __RB_DMA_H_INCLUDED_

#include <stdint.h>

// types for size and offset vars extracted to customize for target buffer size.
typedef uint16_t rb_offset_t;
typedef uint16_t rb_size_t;

struct dma_rb_data_view {
    const uint8_t *data;
    rb_size_t length;
};

// ring buffer struct exposed for static allocation.
// do not access these elements directly.
struct dma_ring_buffer {
    uint8_t * const buffer;
    const rb_size_t buffer_size;
    volatile rb_size_t used;
    volatile rb_offset_t head_offset, tail_offset;
};

int rb_init(struct dma_ring_buffer *rb, const uint8_t  *buffer, rb_size_t buffer_size);
int rb_add(struct dma_ring_buffer *rb, const uint8_t *in, rb_size_t in_size);
int rb_tail_data_view(struct dma_ring_buffer *rb, struct dma_rb_data_view *out_node_view);
int rb_remove_tail (struct dma_ring_buffer *rb);


#endif //__RB_DMA_H_INCLUDED_
