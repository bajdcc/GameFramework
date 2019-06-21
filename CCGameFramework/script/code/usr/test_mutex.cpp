#include "/include/io"
#include "/include/fs"
#include "/include/shell"
#include "/include/proc"
#include "/include/xtoa_itoa"
#include "/include/string"
int main(int argc, char **argv) {
    int i, j;
    put_string("========== [#13 TEST MUTEX] ==========\n");
    shell("touch /pipe/__test_mutex__");
    shell("touch /mutex/__test_mutex__");
    int handle = open("/pipe/__test_mutex__");
    truncate(handle);
    int num = 5;
    for (j = 1; j <= num; j++) {
        i = fork();
        if (i == -1) {
            // CHILD
            int k;
            char* s = malloc(100), * s2 = malloc(10);
            int mutex;
            sleep(1000);
            for (k = 1; k <= 10; k++) {
                mutex = open("/mutex/__test_mutex__");
                read(mutex);
                strcpy(s, "echo [");
                i32toa(j, s2);
                strcat(s, s2);
                strcat(s, "] ");
                strcat(s, "Post number ");
                i32toa(k, s2);
                strcat(s, s2);
                strcat(s, ">> /pipe/__test_mutex__");
                shell(s);
                shell("space 2 >> /pipe/__test_mutex__");
                shell("cat /sys/time >> /pipe/__test_mutex__");
                shell("newline >> /pipe/__test_mutex__");
                close(mutex);
            }
            close(handle);
            return 0;
        }
    }
    // PARENT
    close(handle);
    shell("cat /pipe/__test_mutex__");
    for (j = 0; j < num; j++) {
        wait();
    }
    shell("rm /pipe/__test_mutex__");
    shell("rm /mutex/__test_mutex__");
    put_string("========== [#13 TEST MUTEX] ==========\n");
    return 0;
}