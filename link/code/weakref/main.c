#include <stdio.h>
#include "test.h"
void test(void)
{
    printf("running custom weak ref function!\n");
}

int main()
{
    test_func();
    return 0;
}