#include <stream_rb.h>

int
main ()
{
    uint8_t rb_buffer[512];
    struct stream_ring_buffer rb = {0};
    memset(rb_buffer, 0, sizeof(rb_buffer));
    stream_rb_init(&rb, rb_buffer, sizeof(rb_buffer));
    
    stream_rb_print_state(&rb);
    
    uint8_t buff[256];
    snprintf(buff, sizeof(buff), "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf");
    
    
    //rb_node_t rb_node;

    
    stream_rb_add(&rb, buff, strlen(buff));
    stream_rb_print_state(&rb);
    stream_rb_add(&rb, buff, strlen(buff));
    stream_rb_print_state(&rb);
    stream_rb_add(&rb, buff, strlen(buff));
    stream_rb_print_state(&rb);
    stream_rb_add(&rb, buff, strlen(buff));
    stream_rb_print_state(&rb);
    
    printf("\n");
    
    stream_rb_remove_tail(&rb);
    stream_rb_print_state(&rb);
    stream_rb_remove_tail(&rb);
    stream_rb_print_state(&rb);
    stream_rb_remove_tail(&rb);
    stream_rb_print_state(&rb);
    stream_rb_remove_tail(&rb);
    stream_rb_print_state(&rb);
    
    printf("\n");
    
    stream_rb_add(&rb, buff, strlen(buff));
    stream_rb_print_state(&rb);
    stream_rb_remove_tail(&rb);
    stream_rb_print_state(&rb);
    stream_rb_remove_tail(&rb);
    stream_rb_print_state(&rb);
    
    
    printf ("%u", sizeof("asdf"));

  return 0;
}
