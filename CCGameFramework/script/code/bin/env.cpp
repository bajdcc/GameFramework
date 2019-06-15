#include "/include/shell"
#include "/include/proc"
#include "/include/xtoa_itoa"
#include "/include/string"
int main(int argc, char** argv) {
    int pid = get_pid();
    char* pids = malloc(32);
    i32toa(pid, pids);
    char* cmd = malloc(256);
    strcpy(cmd, "cat /proc/");
    strcat(cmd, pids);
    strcat(cmd, "/path");
    shell(cmd);
    return 0;
}