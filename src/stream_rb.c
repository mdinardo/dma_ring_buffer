/*
    Pre-allocated Circular Ring Buffer implementation for 
        transmitting raw data streams.
    This implementation is not suitable for applications that require
        a one-to-one relationship between the number of data chunks added 
        and nodes in the ring buffer.
    
    The target application in mind during creation is 
        device logging -> DMA writes to a serial device (UART).
    The buffer is designed to minimize interrupt context overhead and complexity
        for DMA interrupts when setting up the next transmit burst.
    
    Data is saved in the buffer within a framing structure; 
        typically there is 1 frame per data chunk added.
    Typically in a ring buffer, when trying to add data that would go past the bounds of the buffer,
    data is filled to the end and wrapped around directly to the start of the buffer.
    This implementation instead will split the data chunk across 2 nodes;
        One extending as far as it can to the end of the buffer,
        and the other is placed at the start of the buffer.
    This minimizes overhead in an interrupt context as it only has to
        load the node information instead of keeping track of a 
        single node's data wrapping around to the beginning.
    
    example with data overrunning end of buffer:
    note that hdr indicates a node's header info,
    and |hdr>x x x x ...| indicates a node with data bytes x x x ....

    add 0xdeadbeef
    new:  ...|hdr>d e a d b e e f|
    buf: [...|hdr>0 0 0 0]   
    | split data across 2 nodes and inserted |
    v                                        v
    buf: [|hdr>b e e f|... |hdr>d e a d|]
*/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "stream_rb.h"

#pragma pack(push)
typedef struct rb_node
{
    stream_rb_offset_t next_offset;
    stream_rb_size_t data_size;
    uint8_t *data[0]; // provides offset to node data without increasing node size
} rb_node_t;
#pragma pack(pop)

typedef struct stream_ring_buffer rb_t;
typedef struct stream_rb_data_view rb_data_view_t;

static inline rb_node_t* offset_to_node(const rb_t *rb, stream_rb_offset_t offset) {
    return (rb_node_t*)(&rb->buffer[offset] );
}

static inline stream_rb_offset_t node_to_offset(const rb_t *rb, const rb_node_t *node) {
    return (stream_rb_offset_t) ( (void*)node - (void*)(&rb->buffer) );
}

static inline stream_rb_size_t size_align_node(stream_rb_size_t x) {
    stream_rb_size_t align_mask = sizeof(rb_node_t) - 1;
    return ((x + align_mask) & ~align_mask);
}

static inline stream_rb_offset_t offset_distance(const rb_t *rb, stream_rb_offset_t start, stream_rb_offset_t end) {
    // (end < start) means that the next node has wrapped to the start of the buffer
    return (end < start) ? 
            (rb->buffer_size - start) + end :
            (end - start);
}

#ifdef STREAM_RING_BUFFER_DEBUG
#include <stdio.h>
#define DEBUG_LOG(fmt, ...) printf(fmt, __VA_ARGS__)
void stream_rb_print_state(const rb_t *rb) {
    printf("rb_t @0x%p\n  buffer @0x%p\n  size: %u\n  used: %u\n  head_offset: %u\n  tail_offset: %u\n", 
        rb,
        rb->buffer,
        rb->buffer_size,
        rb->used,
        rb->head_offset,
        rb->tail_offset
        );
}
void stream_rb_traverse_nodes(const rb_t *rb) {
    printf("Traversing ring buffer @0x%p\n", rb);

    volatile stream_rb_size_t num_nodes, bytes_traversed, data_bytes_traversed;
    rb_node_t *node;
    stream_rb_offset_t node_offset;

    num_nodes = 0;
    bytes_traversed = 0;
    data_bytes_traversed = 0;
    
    node_offset = rb->tail_offset;
    node = offset_to_node(rb, node_offset);

    while(bytes_traversed < rb->used) {

        stream_rb_size_t this_node_size = (stream_rb_size_t) offset_distance(rb, node_offset, node->next_offset);
        printf("Node %u\n  offset: %u\n  node_size: %u\n  data_size: %u\n  data: ", 
            num_nodes, node_offset, this_node_size, node->data_size);
        int i;
        for(i = 0; i < node->data_size; i++) {
            printf("%02X ", ((uint8_t*)node->data)[i]);
        }
        printf("\n", "");

        data_bytes_traversed += node->data_size;
        bytes_traversed += this_node_size;

        num_nodes++;
        node_offset = node->next_offset;
        node = offset_to_node(rb, node_offset);
    }

    printf("Traversal finished\n  num_nodes: %u\n  bytes_traversed: %u\n  data_bytes_traversed: %u\n",
        num_nodes, bytes_traversed, data_bytes_traversed
        );
}
#else
#define DEBUG_LOG(fmt, ...)
#endif

int stream_rb_init(rb_t *rb, const uint8_t *buffer, stream_rb_size_t buffer_size) {
    DEBUG_LOG("Init rb @0x%p with buffer @0x%p of size %u\n",
        rb, buffer, buffer_size);
    // override const qualifier for init only
    *(uint8_t**)(&rb->buffer) = buffer;
    *(stream_rb_size_t*)(&rb->buffer_size) = buffer_size;
    memset (rb->buffer, 0, rb->buffer_size);
    rb->used = 0;
    rb->head_offset = 0;
    rb->tail_offset = 0;

    return 0;
}

int stream_rb_remove_tail (rb_t * rb) {
    // case that this function is called when the rb is empty
    if (rb->used <= 0) {
        DEBUG_LOG("rb is empty! no tail to remove.\n", "");
        return -1;
    }
        
    rb_node_t *tail;
    stream_rb_offset_t tail_offset, next_tail_offset;
    stream_rb_size_t bytes_to_free;

    tail_offset = rb->tail_offset;
    tail = offset_to_node(rb, tail_offset);
    next_tail_offset = tail->next_offset;

    // (next_offset < tail) offset means that the next node has wrapped to the start
    // so current offset to end should be freed.
    // else free the delta between new and current tail offsets.
    // bytes_to_free = (next_tail_offset < tail_offset) ? 
    //                     (rb->buffer_size - tail_offset) :
    //                     (next_tail_offset - tail_offset);
    bytes_to_free = offset_distance(rb, tail_offset, next_tail_offset);
    
    DEBUG_LOG("rb removing node:\n  used: %u\n  tail_offset: %u\n  bytes_freed: %u\n  next_tail_offset: %u\n",
        rb->used,
        rb->tail_offset,
        bytes_to_free,
        next_tail_offset
        );
        
    rb->tail_offset = next_tail_offset;
    rb->used -= bytes_to_free;
    
    return 0;
}

/*
Dumbly insert new node at specified offset and copies data.
Do not modify the ring buffer journaling.
If out_next_offset is not NULL, the next head offset is written there.

this function assumes that the calling function has ensured that
data will not be copied out of bounds of the rb buffer.
*/
static int stream_rb_soft_add_node(const rb_t *rb, stream_rb_offset_t offset, const uint8_t *in, stream_rb_size_t in_size, stream_rb_offset_t *out_next_offset) {
    int ret = 0;
    rb_node_t *head;
    stream_rb_size_t requested_size;
    stream_rb_offset_t next_offset;
    
    head = offset_to_node(rb, offset);
    requested_size = sizeof(rb_node_t) + size_align_node(in_size);
    
    // compute next head offset location.  wrap to start if necessary
    next_offset = (offset + requested_size);
    next_offset = (next_offset < rb->buffer_size) ? next_offset : 0;
    
    DEBUG_LOG("Inserting node and copying data to offset %u\n", offset);
    // write node header info and copy data over.
    head->next_offset = next_offset;
    head->data_size = in_size;
    memcpy((void*)(&head->data), in, in_size);
    
    
    if(NULL != out_next_offset) *out_next_offset = next_offset;
    
    return 0;
}

/*
Add data chunk into ring buffer.
data chunk is split across one or more nodes as necessary for wrapping.
*/
int stream_rb_add(rb_t *rb, const uint8_t * in, stream_rb_size_t in_size) {
    // use temp node and offset variables in function.
    // rb head_offset and # used bytes control add operation.
    // only copy these to the rb struct when everything is inserted correctly
    // to preserve rb integrity.
    // head node can be edited and new nodes inserted whenever;
    // changes are ignored until the rb struct vars are updated.
    
    int ret = 0;
    rb_node_t *tmp_node;
    stream_rb_offset_t tmp_head_offset;
    stream_rb_size_t requested_size;
    stream_rb_size_t prewrap_in_size;
    bool wrap_data = false;
    
    tmp_head_offset = rb->head_offset;
    
    requested_size = sizeof(rb_node_t) + size_align_node(in_size);
    
    // (head_offset + requested_size) = buffer_size 
    // => perfect fit, no wrap.  near end of function, head_offset moved to 0
    
    // (head_offset + requested_size) > buffer_size
    if((tmp_head_offset + requested_size) > rb->buffer_size) {
        requested_size += sizeof(rb_node_t);
        wrap_data = true;
    }
    
    // fail if not enough memory; return non-zero
    if(requested_size > (rb->buffer_size - rb->used)) {
        DEBUG_LOG("rb Requested size too big!  %u > %u\n", requested_size, (rb->buffer_size - rb->used));
        ret = -1;
        goto stream_rb_add_exit;
    }
    
    DEBUG_LOG("Wrapping data? %u\n", wrap_data);
    // if we have to wrap to the start of the buffer,
    // insert the prewrap node, and modify the in data vars
    // for the postwrap node insertion.
    if(wrap_data) {
        prewrap_in_size = (rb->buffer_size - rb->head_offset) - sizeof(rb_node_t);
        
        stream_rb_soft_add_node(rb, tmp_head_offset, in, prewrap_in_size, &tmp_head_offset);
        
        in += prewrap_in_size;
        in_size -= prewrap_in_size;
    }
    
    stream_rb_soft_add_node(rb, tmp_head_offset, in, in_size, &tmp_head_offset);
    
    DEBUG_LOG("rb adding data:\n  current_head_offset: %u\n  requested_size: %u\n  new_head_offset: %u\n",
        rb->head_offset, requested_size, tmp_head_offset);
    //after inserting node(s), move head to new head node.
    //might have to wrap it to the start of the buffer.
    rb->head_offset = (tmp_head_offset < rb->buffer_size) ? tmp_head_offset : 0;
    rb->used += requested_size;

stream_rb_add_exit:
    return ret;
}

void stream_rb_tail_data_view(const rb_t *rb, rb_data_view_t *out_node_view) {
    rb_node_t *tail = offset_to_node(rb, rb->tail_offset);
    *(uint8_t**)(&out_node_view->data) = (uint8_t*)tail->data;
    *(stream_rb_size_t*)(&out_node_view->length) = tail->data_size;
}
