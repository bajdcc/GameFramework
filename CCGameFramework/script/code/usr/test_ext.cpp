#include "/include/shell"
void run(char* cmd) {
    put_string("# ");
    put_string(cmd);
    put_string("\n");
    shell(cmd);
    put_string("\n");
    shell("sleep 1");
}
int main(int argc, char** argv) {
    put_string("========== [#26 TEST EXT] ==========\n");
    run("tree /ext");
    run("cat /ext/web/func/test");
    run("cat /ext/web/func/file/__name__");
    run("cat /ext/web/func/file/__version__");
    run("ipc ext_web file/__version__");
    run("echo test 1234 > /ext/web/func/file/test_file");
    run("cat /ext/web/func/file/test_file");
    run("ls /ext/web/func/file");
    run("mklink /ext/web/func/file/test_link /");
    run("rm /ext/web/func/file/test_file");
    run("ipc ext_web file:tree");
    run("ipc ext_win file/__call__:sys:computer_name");
    run("ipc ext_win file/__call__:sys:user_name");
    run("api_ipc win sys user_name");
    put_string("========== [#26 TEST EXT] ==========\n");
    return 0;
}