# 信号的一些使用

## 注册信号处理函数

### signal
> 这种方式已经不推荐使用，只有用于将某些信号的处理方式设置为SIG_IGN或SIG_DFL时，才使用这个函数，否则， ***如果使用自定义的处理函数时，第一次响应之后系统会默认将对应的信号处理方式设置为默认的方式***，在 POSIX.1中解决了这个问题，使用signaction函数代替signal 函数，但是这里需要特殊处理一下，用于启用POSIX.1特性

１. 在对应的源文件任何#命令之前使用#define _POSIX_SOURCE 启用这个特性

２. 在对应的源文件任何#命令之前使用#define _POSIX_C_SOURCE 启用这个特性,这个宏需要定义几个特定的值用于支持不同的POSIX.X特性，例如POSIX.1对应199506L，这些值其实就是对应的版本成为标准的时间。
示例程序如下
```
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199506L
#endif

或

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif

＃include <stdio.h>


...
```
３. 在gcc　编译参数中使用-D的方式引入上述宏定义，示例程序如下:
```
gcc -D_POSIX_C_SOURCE=199506L

gcc -D_POSIX_SOURCE
```
