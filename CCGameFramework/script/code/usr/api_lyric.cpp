#include "/include/proc"
#include "/include/io"
#include "/include/memory"
#include "/include/shell"
int main(int argc, char **argv) {
    if (argc <= 1) {
        return 0;
    }
    char* cmd = malloc(256);
    strcpy(cmd, "cat /http/music.163.com/api/song/media?id=");
    strcat(cmd, argv[1]);
    strcat(cmd, " | /usr/api_json");
    shell(cmd);
    return 0;
}