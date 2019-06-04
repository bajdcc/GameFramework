#include "/include/proc"
#include "/include/io"
#include "/include/memory"
#include "/include/shell"
int main(int argc, char **argv) {
    shell("/usr/http_get v1.hitokoto.cn | /usr/api_json");
    return 0;
}