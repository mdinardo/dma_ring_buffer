#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <stream_rb.h>

#ifdef STREAM_RING_BUFFER_DEBUG
#define TEST_RB_PRINT_STATE(p_rb) (stream_rb_print_state(p_rb))
#define TEST_RB_TRAVERSE_NODES(p_rb) (stream_rb_traverse_nodes(p_rb))
#else
#define TEST_RB_PRINT_STATE(p_rb)
#define TEST_RB_TRAVERSE_NODES(p_rb) 
#endif

int test_can_wrap() {
    uint8_t rb_buffer[512];
    struct stream_ring_buffer rb = {0};
    memset(rb_buffer, 0, sizeof(rb_buffer));
    stream_rb_init(&rb, rb_buffer, sizeof(rb_buffer));
    
    TEST_RB_PRINT_STATE(&rb);
    
    uint8_t buff[256];
    snprintf(buff, sizeof(buff), "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf");

    
    stream_rb_add(&rb, buff, strlen(buff));
    TEST_RB_PRINT_STATE(&rb);
    stream_rb_add(&rb, buff, strlen(buff));
    TEST_RB_PRINT_STATE(&rb);
    stream_rb_add(&rb, buff, strlen(buff));
    TEST_RB_PRINT_STATE(&rb);
    stream_rb_add(&rb, buff, strlen(buff));
    TEST_RB_PRINT_STATE(&rb);
    
    printf("\n");
    
    stream_rb_remove_tail(&rb);
    TEST_RB_PRINT_STATE(&rb);
    stream_rb_remove_tail(&rb);
    TEST_RB_PRINT_STATE(&rb);
    stream_rb_remove_tail(&rb);
    TEST_RB_PRINT_STATE(&rb);
    stream_rb_remove_tail(&rb);
    TEST_RB_PRINT_STATE(&rb);
    
    printf("\n");
    
    stream_rb_add(&rb, buff, strlen(buff));
    TEST_RB_PRINT_STATE(&rb);

    TEST_RB_TRAVERSE_NODES(&rb);

    stream_rb_remove_tail(&rb);
    TEST_RB_PRINT_STATE(&rb);
    stream_rb_remove_tail(&rb);
    TEST_RB_PRINT_STATE(&rb);
    
    return 0;
}

int test_wrap_perfect_fit() {
    printf("\n*** test_wrap_perfect_fit ***\n");
    uint8_t rb_buffer[32];
    struct stream_ring_buffer rb = {0};
    memset(rb_buffer, 0, sizeof(rb_buffer));
    stream_rb_init(&rb, rb_buffer, sizeof(rb_buffer));
    
    TEST_RB_PRINT_STATE(&rb);
    
    uint8_t buff[32];
    snprintf(buff, sizeof(buff), "1234567");

    stream_rb_add(&rb, buff, strlen(buff)+1);
    stream_rb_add(&rb, buff, strlen(buff)+1);
    TEST_RB_PRINT_STATE(&rb);

    stream_rb_remove_tail(&rb);
    TEST_RB_PRINT_STATE(&rb);

    stream_rb_add(&rb, buff, strlen(buff)+1);
    TEST_RB_PRINT_STATE(&rb);

    TEST_RB_TRAVERSE_NODES(&rb);

    return 0;
}

int main() {
  test_can_wrap();

  test_wrap_perfect_fit();

  return 0;
}
