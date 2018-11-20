#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <unity.h>

#include <stream_rb.h>

void test_basic(void) {
    printf("basic test; doing nothing\n");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_basic);
    return UNITY_END();
}