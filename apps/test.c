#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h> // 包含 sched_yield() 函数的头文件

void *thread3(void *arg)
{
    sched_yield(); // 让出CPU，使得其他线程有机会执行
    printf("thread3\n");
    return NULL;
}

void *thread2(void *arg)
{
    pthread_t tid;
    pthread_create(&tid, NULL, thread3, NULL); // 创建线程3
    sched_yield(); // 让出CPU，使得其他线程有机会执行
    printf("thread2\n");
    return NULL;
}

void *thread1(void *arg)
{
    pthread_t tid;
    pthread_create(&tid, NULL, thread2, NULL); // 创建线程2
    sched_yield(); // 让出CPU，使得其他线程有机会执行
    printf("thread1\n");
    sched_yield(); // 让出CPU，使得其他线程有机会执行
    return NULL;
}

int main(void)
{
    pthread_t tid;
    pthread_create(&tid, NULL, thread1, NULL); // 创建线程1
    pthread_exit(NULL); // 等待线程1执行完毕
    return 0;
}
