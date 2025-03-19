#include <unistd.h>
#include <stdio.h>

int main() {
    for(int i = 5; i >= 1; i --) {
        printf("mysleep is running,after %ds will finish\n", i);
        sleep(1);
    }
    printf("mysleep is finished\n");
    return 0;
}