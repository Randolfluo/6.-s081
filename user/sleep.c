#include "kernel/types.h"   //数据类型头文件
#include "user/user.h"      //系统调用


int 
main(int argc, char* argv[])
{
    if(argc != 2)
    {
        char str[15] = "Args must be 2!";
        write(2, str, strlen(str)); 
        //0标准输入，1标准输出，2标准错误
        exit(1);
    }
    sleep(atoi(argv[1]));
    exit(0);
}

