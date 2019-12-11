#include "/include/arg"
#include "/include/shell"
#include "/include/format"
#include "/include/readfile"
#include "/include/io"
void run(char* cmd) {
    shell(cmd);
    free(cmd);
}
int main(int argc, char** argv) {
    if (argc > 1) {
        char* args = arg_string(1, argc, argv);
        char* uuid; int len;
        if (readfile("/dev/uuid", &uuid, &len) == 0) {
            run(format("touch /fifo/ipc_req_%s", uuid));
            run(format("touch /fifo/ipc_res_%s", uuid));
            run(format("mklink /ipc/req_%s /fifo/ipc_req_%s hide", uuid, uuid));
            run(format("mklink /ipc/res_%s /fifo/ipc_res_%s hide", uuid, uuid));
            run(format("echo %s > /ipc/service", uuid));
            run(format("echo %s > /ipc/req_%s", args, uuid));
            run(format("cat /ipc/res_%s", uuid));
        }
        free(uuid);
        free(args);
    }
    return 0;
}