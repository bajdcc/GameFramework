#include "/include/shell"
#include "/include/io"
#include "/include/proc"
int main(int argc, char **argv) {
    for (;;) {
        put_string("\r当前时间：");
        shell("cat /sys/time");
        sleep(100);
    }
    return 0;
}