#include "/include/proc"
#include "/include/io"
#include "/include/memory"
#include "/include/shell"
int main(int argc, char **argv) {
    shell("cat /http/v1.hitokoto.cn | /usr/api_json");
    return 0;
}