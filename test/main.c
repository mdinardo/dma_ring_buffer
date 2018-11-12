#include <rb.h>

int
main ()
{
    uint8_t rb_buffer[512];
    rb_t rb = {0};
    memset(rb_buffer, 0, sizeof(rb_buffer));
    rb_init(&rb, rb_buffer, sizeof(rb_buffer));
    
    rb_print_state(&rb);
    
    uint8_t buff[256];
    snprintf(buff, sizeof(buff), "asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf");
    
    
    //rb_node_t rb_node;

    
    rb_add(&rb, buff, strlen(buff));
    rb_print_state(&rb);
    rb_add(&rb, buff, strlen(buff));
    rb_print_state(&rb);
    rb_add(&rb, buff, strlen(buff));
    rb_print_state(&rb);
    rb_add(&rb, buff, strlen(buff));
    rb_print_state(&rb);
    
    printf("\n");
    
    rb_remove_tail(&rb);
    rb_print_state(&rb);
    rb_remove_tail(&rb);
    rb_print_state(&rb);
    rb_remove_tail(&rb);
    rb_print_state(&rb);
    rb_remove_tail(&rb);
    rb_print_state(&rb);
    
    printf("\n");
    
    rb_add(&rb, buff, strlen(buff));
    rb_print_state(&rb);
    rb_remove_tail(&rb);
    rb_print_state(&rb);
    rb_remove_tail(&rb);
    rb_print_state(&rb);
    
    
    int x = RB_NODE_TO_OFFSET(&rb, 6 + (void*)&rb);
    printf ("%u", sizeof("asdf"));

  return 0;
}
