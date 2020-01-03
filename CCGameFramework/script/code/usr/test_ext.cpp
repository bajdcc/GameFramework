#include "/include/shell"
void run(char* cmd) {
    put_string("# ");
    put_string(cmd);
    put_string("\n");
    shell(cmd);
    put_string("\n");
}
int main(int argc, char** argv) {
    put_string("========== [#26 TEST EXT] ==========\n");
    run("tree /ext");
    run("cat /ext/web/func/version");
    run("cat /ext/web/func/test"); 
    put_string("========== [#26 TEST EXT] ==========\n");
    return 0;
}