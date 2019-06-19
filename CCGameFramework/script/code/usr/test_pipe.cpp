#include "/include/io"
#include "/include/fs"
#include "/include/shell"
#include "/include/proc"
int main(int argc, char **argv) {
    int i;
    put_string("========== [#11 TEST PIPE] ==========\n");
    shell("touch /pipe/__test_pipe__");
    int handle = open("/pipe/__test_pipe__");
    truncate(handle);
    i = fork();
    if (i == -1) {
        // CHILD
        shell("echo pipe designed by bajdcc > /pipe/__test_pipe__");
        sleep(500);
        shell("echo , yeah! >> /pipe/__test_pipe__");
        close(handle);
    } else {
        // PARENT
        shell("cat /pipe/__test_pipe__");
        wait();
    }
    return 0;
}