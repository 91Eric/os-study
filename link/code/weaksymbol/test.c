#include <stdio.h>
#include "test.h"

void __attribute__((weak)) weak_func(void)
{
    printf("defualt weak func is running!\n");
}

void test_func(void)
{
    weak_func();
}