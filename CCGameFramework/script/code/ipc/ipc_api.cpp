#include "/include/proc"
#include "/include/shell"
#include "/include/readfile"
#include "/include/string"
#include "/include/format"
#include "/include/io"
void show(char* cmd) {
    put_string(cmd);
    free(cmd);
}
void run(char* cmd) {
    put_string(cmd); put_char('\n');
    shell(cmd);
    free(cmd);
}
void pipe() {
    int c;
    while ((c = io_pipe()) != -1) {
        put_char((char)c);
    }
}
int main(int argc, char** argv) {
    shell("touch /fifo/ipc_service_api");
    shell("mklink /ipc/service_api /fifo/ipc_service_api hide");
    put_string("service api started\n");
    path_add("/usr");
    char* data; int len;
    for (;;) {
        if (recv_signal() == 9) break;
        if (readfile("/ipc/service_api", &data, &len) != 0) {
            sleep(100);
            continue;
        }
        show(format("request come: %s\n", data));
        if (fork() == -1) {
            redirect_to_parent();
            char* uuid = data;
            char* path = format("/ipc/req_%s", uuid);
            show(format("reading: %s\n", path));
            if (readfile(path, &data, &len) == 0) {
                char* t = data;
                char* p = strchr(data, ' ');
                if (!p) {
                    p = " ";
                }
                *p = '\0';
                show(format("shell /usr/%s_%s\n", t, p + 1));
                exec_service_toggle_mode(1);
                run(format("/usr/%s_%s > /ipc/res_%s", t, p + 1, uuid));
                exec_service_toggle_mode(0);
                free(data);
            }
            show(format("request done: %s\n", uuid));
            free(uuid);
            free(path);
            exit(0);
        }
        pipe();
        free(data);
    }
    shell("rm /fifo/ipc_service_api");
    shell("rm /ipc/service_api");
    put_string("service api stopped\n");
    return 0;
}