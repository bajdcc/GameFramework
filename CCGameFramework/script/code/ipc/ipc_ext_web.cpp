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
    shell("load_ext CCOS_EXT_WEB");
    shell("echo [*] 启动IPC_EXT_WEB服务！ > /ipc/ipc_service_ext_web_log");
    shell("echo  扩展名称： >> /ipc/ipc_service_ext_web_log");
    shell("cat /ext/web/func/file/__name__ >> /ipc/ipc_service_ext_web_log");
    shell("echo  版本： >> /ipc/ipc_service_ext_web_log");
    shell("cat /ext/web/func/file/__version__ >> /ipc/ipc_service_ext_web_log");
    shell("cat /ipc/ipc_service_ext_web_log > /fifo/sys_entry_console");
    shell("rm /ipc/ipc_service_ext_web_log");
    shell("touch /fifo/ipc_service_ext_web");
    shell("mklink /ipc/service_ext_web /fifo/ipc_service_ext_web hide");
    put_string("service ext_web started\n");
    shell("touch /mutex/__ipc_service_ext_web_mutex__");
    path_add("/bin");
    char* data; int len;
    for (;;) {
        if (recv_signal() == 9) break;
        if (readfile("/ipc/service_ext_web", &data, &len) != 0) {
            sleep(100);
            continue;
        }
        if (recv_signal() == 9) break;
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
                exec_service_toggle_mode(1);
                run(format("cat /ext/web/func/%s > /ipc/res_%s", p + 1, uuid));
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
    shell("rm /fifo/ipc_service_ext_web");
    shell("rm /ipc/service_ext_web");
    shell("rm /mutex/__ipc_service_ext_web_mutex__");
    put_string("waiting for process exit...\n");
    send_signal(get_pid(), 9);
    wait_children();
    put_string("service ext_web stopped\n");
    return 0;
}