#include <unistd.h>
#include <stdio.h>

int main() {
    printf("mysleep is running,after 5s will finish\n");
    sleep(5);    
    printf("mysleep is finished\n");
    return 0;
}