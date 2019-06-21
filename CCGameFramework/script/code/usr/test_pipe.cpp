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
        sleep(100);
        shell("echo pipe designed by bajdcc > /pipe/__test_pipe__");
        sleep(100);
        shell("echo , yeah! >> /pipe/__test_pipe__");
        shell("newline >> /pipe/__test_pipe__");
        sleep(100);
        shell("api_hitokoto >> /pipe/__test_pipe__");
        close(handle);
    } else {
        // PARENT
        close(handle);
        shell("cat /pipe/__test_pipe__");
        wait();
        newline();
        shell("rm /pipe/__test_pipe__");
        put_string("========== [#11 TEST PIPE] ==========\n");
    }
    return 0;
}