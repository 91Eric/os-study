#include <stdio.h>

extern int shared;
extern void swap(int *a, int *b);

int main(int argc, char const *argv[])
{
    int a = 4;
    swap(&a, &shared);
    return 0;
}
