#include "kernel/types.h"
#include "user/user.h"

int isPrime(int n) {
    int i;
    if (n < 2)
        return 0;
    for (i = 2; i <= n / 2; i++) {
        if (n % i == 0)
            return 0;
    }
    return 1;
}

void func(int pipe_out) {
    int x;
    if (read(pipe_out, &x, sizeof(int)) == 0)
        return; // 如果读不到数据，直接返回

    if (isPrime(x)) {
        int p[2];
        pipe(p);
        if (fork() == 0) { // 子进程
            close(pipe_out);
            close(p[1]);
            printf("prime %d\n", x);
            func(p[0]);
            close(p[0]);
            exit(0);
        } else { // 父进程
            close(p[0]);
            int xx;
            while (read(pipe_out, &xx, sizeof(int)) > 0) {
                write(p[1], &xx, sizeof(int));
            }
            close(pipe_out);
            close(p[1]);
            wait(0); // 等待子进程结束
            exit(0);
        }
    } else {
        func(pipe_out);
    }
}

int main(int argc, char *argv[]) {
    int p[2];
    pipe(p);

    if (argc != 1) {
        char str[] = "Args must be 1!";
        write(2, str, strlen(str));
        exit(1);
    }

    for (int i = 2; i <= 35; i++) {
        write(p[1], &i, sizeof(int));
    }
    close(p[1]);
    func(p[0]);
    close(p[0]);
    exit(0);
}
