#include <stdio.h>
static __attribute__((weakref("test"))) void weak_ref(void);
void test_func(void)
{
    if(weak_ref){
        weak_ref();
    }
    else{
        printf("weak ref function is null\n");
    }
}