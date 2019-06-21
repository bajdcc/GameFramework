#include "/include/shell"
int main(int argc, char** argv) {
    int i;
    put_string("========== [#12 TEST LUA] ==========\n");
    shell("cat test_lua.txt | bat");
    put_string("========== [#12 TEST LUA] ==========\n");
    return 0;
}