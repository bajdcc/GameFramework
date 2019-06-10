#include "/include/shell"
#include "/include/io"
#include "/include/proc"
int main(int argc, char **argv) {
    for (;;) {
        put_string("\r运行时间：");
        shell("cat /sys/uptime");
        sleep(100);
    }
    return 0;
}