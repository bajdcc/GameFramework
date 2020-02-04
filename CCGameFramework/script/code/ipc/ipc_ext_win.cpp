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
    shell("load_ext CCOS_EXT_WIN");
    shell("echo [*] 启动IPC_EXT_WIN服务！ > /ipc/ipc_service_ext_win_log");
    shell("echo  扩展名称： >> /ipc/ipc_service_ext_win_log");
    shell("cat /ext/win/func/file/__name__ >> /ipc/ipc_service_ext_win_log");
    shell("echo  版本： >> /ipc/ipc_service_ext_win_log");
    shell("cat /ext/win/func/file/__version__ >> /ipc/ipc_service_ext_win_log");
    shell("cat /ipc/ipc_service_ext_win_log > /fifo/sys_entry_console");
    shell("rm /ipc/ipc_service_ext_win_log");
    shell("touch /fifo/ipc_service_ext_win");
    shell("mklink /ipc/service_ext_win /fifo/ipc_service_ext_win hide");
    put_string("service ext_win started\n");
    shell("touch /mutex/__ipc_service_ext_win_mutex__");
    path_add("/bin");
    char* data; int len;
    for (;;) {
        if (recv_signal() == 9) break;
        if (readfile("/ipc/service_ext_win", &data, &len) != 0) {
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
                exec_service_toggle_mode(1);
                run(format("cat /ext/win/func/%s > /ipc/res_%s", p + 1, uuid));
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
    shell("rm /fifo/ipc_service_ext_win");
    shell("rm /ipc/service_ext_win");
    shell("rm /mutex/__ipc_service_ext_win_mutex__");
    put_string("waiting for process exit...\n");
    send_signal(get_pid(), 9);
    wait_children();
    put_string("service ext_win stopped\n");
    return 0;
}