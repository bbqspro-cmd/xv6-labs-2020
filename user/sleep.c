#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("usage: %s <ticks>\n", argv[0]);
        exit(1);//程序异常、参数错误、执行失败
    }
    // 获取命令行传进来的参数，假如输入： sleep 3
    // argv[0] = sleep，第一个参数是程序名称 sleep。
    // argv[1] = 3，第二个参数是需要暂停多少个 tick
    char *arg = argv[1];
    int ticks = atoi(arg);
    printf("ready to sleep for %d ticks...\n",ticks);
    // 调用系统提供的函数进行暂停
    sleep(ticks);
    printf("sleep finished. Goodbys!\n");
    exit(0);//程序执行成功、无错误正常结束
}