#include <stdio.h>
#include "test.h"

void weak_func(void)
{
    printf("custom strong func override!\n");
}

int main()
{
    test_func();
    return 0;
}