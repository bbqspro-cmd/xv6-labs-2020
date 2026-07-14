#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    int i=100;
    // 每个管道描述符长度为 2，其中，0 用于读数据，1 用于写数据
    //前者用于父进程向子进程通信
    int fd_p2c[2], fd_c2p[2];
    pipe(fd_p2c);
    pipe(fd_c2p);

    if(fork() == 0){  //children
        sleep(10);
        printf("children process,pid = %d, i = %d, i addr is %pn, fd_p2c addr is %pn, fd_c2p addr is %p\r\n",getpid(),i,&i,&fd_p2c,&fd_c2p);

        int port_read = fd_p2c[0];  //子读父
        int port_write = fd_c2p[1];
        char content_receive[1024] = {0};
        char content_send[1024] = {"daddy!\n"};

        read(port_read, content_receive, sizeof(content_receive));
        printf("child received: %s", content_receive);
        printf("%d: received ping\n\n",getpid());
        write(port_write, content_send, sizeof(content_send));
        sleep(10);
        exit(0);
    }else{  //parent
        i = 101;
        printf("here's parent process, pid = %d, i = %d, i addr is %pn, fd_p2c addr is %pn, fd_c2p addr is %p\r\n", getpid(), i, &i, &fd_p2c, &fd_c2p);
        int port_read = fd_c2p[0]; //父读子
        int port_write = fd_p2c[1];

        char content_receive[1024] = {0};
        char content_send[1024] = {"call me daddy please \n"};

        write(port_write, content_send, sizeof(content_send));

        read(port_read, content_receive, sizeof(content_receive));
        printf("parent receive:%s", content_receive);
        printf("%d: receive pong\n\n", getpid());

        sleep(10);
        exit(0);
    }

}