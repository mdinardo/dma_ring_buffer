#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <unity.h>

#include <stream_rb.h>

#ifdef STREAM_RING_BUFFER_DEBUG
#define TEST_RB_PRINT_STATE(p_rb) (stream_rb_print_state(p_rb))
#define TEST_RB_TRAVERSE_NODES(p_rb, p_stats) (stream_rb_traverse_nodes(p_rb, p_stats))
#else
#define TEST_RB_PRINT_STATE(p_rb)
#define TEST_RB_TRAVERSE_NODES(p_rb, p_stats) 
#endif

#define TEST_ASSERT_ZERO(x) TEST_ASSERT_EQUAL(0, (x))

void test_can_add() {
    uint8_t rb_buffer[512] = {0};
    struct stream_ring_buffer rb = {0};
    stream_rb_init(&rb, rb_buffer, sizeof(rb_buffer));

    int num_to_add = 4;

    TEST_ASSERT_EQUAL(0, stream_rb_add(&rb, &num_to_add, sizeof(num_to_add)));

    TEST_RB_TRAVERSE_NODES(&rb, NULL);
    printf("\n");
    // stream_rb_add(&rb, &num_to_add, sizeof(num_to_add));
}

void test_can_remove() {
    uint8_t rb_buffer[16] = {0};
    struct stream_ring_buffer rb = {0};
    stream_rb_init(&rb, rb_buffer, sizeof(rb_buffer));

    int num_to_add = 4;

    TEST_ASSERT_EQUAL(0, stream_rb_add(&rb, &num_to_add, sizeof(num_to_add)));

    TEST_ASSERT_EQUAL(0, stream_rb_remove_tail(&rb));
}

void test_can_wrap() {
    uint8_t rb_buffer[512] = {0};
    struct stream_ring_buffer rb = {0};
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

    TEST_RB_TRAVERSE_NODES(&rb, NULL);

    stream_rb_remove_tail(&rb);
    TEST_RB_PRINT_STATE(&rb);
    stream_rb_remove_tail(&rb);
    TEST_RB_PRINT_STATE(&rb);
    
}

void test_wrap_perfect_fit() {
    uint8_t rb_buffer[32] = {0};
    struct stream_ring_buffer rb = {0};
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

    TEST_RB_TRAVERSE_NODES(&rb, NULL);
}

void test_wrap_does_not_corrupt_data(void) {
    // header size: 4 @32bit , 8 @64bit
    uint8_t rb_buffer[32] = {0};
    struct stream_ring_buffer rb = {0};
    struct stream_rb_data_view data_view = {0};
    stream_rb_init(&rb, rb_buffer, sizeof(rb_buffer));

    char write_data[16] = {0};
    char read_data[16] = {0};
    strcpy(write_data, "123456789abcde");

    stream_rb_add(&rb, write_data, strlen(write_data) + 1);
    printf("%s\n", data_view.data);
    stream_rb_remove_tail(&rb);

    // next data chunk should be split across 2 nodes
    stream_rb_add(&rb, write_data, strlen(write_data) + 1);

    stream_rb_tail_data_view(&rb, &data_view);
    strncpy(&read_data[0], data_view.data, data_view.length);
    stream_rb_remove_tail(&rb);

    stream_rb_tail_data_view(&rb, &data_view);
    strncpy(&read_data[strlen(read_data)], data_view.data, data_view.length);
    stream_rb_remove_tail(&rb);

    printf("%s\n", write_data);
    printf("%s\n", read_data);
    TEST_ASSERT_EQUAL(0, memcmp(write_data, read_data, sizeof(write_data)));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_can_add);
    RUN_TEST(test_can_remove);
    RUN_TEST(test_wrap_does_not_corrupt_data);
    return UNITY_END();
}