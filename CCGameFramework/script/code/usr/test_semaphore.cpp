#include "/include/io"
#include "/include/fs"
#include "/include/shell"
#include "/include/proc"
#include "/include/xtoa_itoa"
#include "/include/string"
int main(int argc, char **argv) {
    int i, j;
    put_string("========== [#14 TEST SEMAPHORE] ==========\n");
    shell("touch /pipe/__test_semaphore__");
    shell("touch /semaphore/__test_semaphore__");
    int handle = open("/pipe/__test_semaphore__");
    truncate(handle);
    int num = 5;
    for (j = 1; j <= num; j++) {
        i = fork();
        if (i == -1) {
            // CHILD
            int k;
            char* s = malloc(100), * s2 = malloc(10);
            int semaphore;
            sleep(1000);
            for (k = 1; k <= 10; k++) {
                semaphore = open("/semaphore/__test_semaphore__:2");
                read(semaphore);
                strcpy(s, "echo [");
                i32toa(j, s2);
                strcat(s, s2);
                strcat(s, "] ");
                strcat(s, "Post number ");
                i32toa(k, s2);
                strcat(s, s2);
                strcat(s, ">> /pipe/__test_semaphore__");
                shell(s);
                shell("space 2 >> /pipe/__test_semaphore__");
                shell("cat /sys/time >> /pipe/__test_semaphore__");
                shell("newline >> /pipe/__test_semaphore__");
                close(semaphore);
            }
            close(handle);
            return 0;
        }
    }
    // PARENT
    close(handle);
    shell("cat /pipe/__test_semaphore__");
    for (j = 0; j < num; j++) {
        wait();
    }
    shell("rm /pipe/__test_semaphore__");
    shell("rm /mutex/__test_semaphore__");
    put_string("========== [#14 TEST SEMAPHORE] ==========\n");
    return 0;
}