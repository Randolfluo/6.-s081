#include "kernel/types.h"   
#include "user/user.h"      


int 
main(int argc, char* argv[])
{
    int p1[2], p2[2];
    int pid;
    pipe(p1);
    pipe(p2);

    if(argc != 1)
    {
        char str[15] = "Args must be 1!";
        write(2, str, strlen(str)); 
        exit(1);
    }

    if((pid = fork()) == 0) //子进程
    {
        close(p1[1]);   //关闭不使用的文件描述符
        close(p2[0]);
        char arr2[10] = "pong";
        char buf[10];
        int n;
        n = read(p1[0], buf, 4);
        buf[n] = '\0';
        printf("%d: received %s\n", getpid(), buf);
        write(p2[1], arr2, strlen(arr2));
        exit(0);
    }
    else {                          //父进程
        close(p1[0]);   //关闭不使用的文件描述符
        close(p2[1]);   
        char arr1[10] = "ping";
        char buf[10];
        int n;
        write(p1[1], arr1, strlen(arr1));
        n = read(p2[0], buf, 4);
        buf[n] = '\0';
        printf("%d: received %s\n", getpid(), buf);
    }

    exit(0);
}