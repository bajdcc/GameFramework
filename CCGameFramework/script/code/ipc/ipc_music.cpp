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
    shell("echo [*] 启动IPC_MUSIC服务！ > /fifo/sys_entry_console");
    shell("touch /fifo/ipc_service_music");
    shell("mklink /ipc/service_music /fifo/ipc_service_music hide");
    put_string("service music started\n");
    shell("touch /mutex/__ipc_service_music_mutex__");
    path_add("/bin");
    char* data; int len;
    for (;;) {
        if (recv_signal() == 9) break;
        if (readfile("/ipc/service_music", &data, &len) != 0) {
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
                char* p2 = strchr(p + 1, ' ');
                if (p2) {
                    *p2 = '\0';
                }
                else {
                    p2 = " ";
                }
                int suc = 1;
                if (strcmp(p + 1, "list") == 0) {
                    if (fork() == -1) {
                        redirect_to_parent();
                        path_add("/usr");
                        shell("cat /ipc/__ipc_service_music_pid__ | signal 9");
                        int mutex = open("/mutex/__ipc_service_music_mutex__");
                        read(mutex);
                        run(format("echo %d > /ipc/__ipc_service_music_pid__", get_pid()));
                        run(format("window_163 %s > /bat/%d", p2 + 1, get_pid()));
                        run(format("rm /ipc/__ipc_service_music_pid__"));
                        close(mutex);
                        exit(0);
                    }
                }
                else if(strcmp(p + 1, "list2") == 0) {
                    if (fork() == -1) {
                        redirect_to_parent();
                        path_add("/usr");
                        shell("cat /ipc/__ipc_service_music_pid__ | signal 9");
                        int mutex = open("/mutex/__ipc_service_music_mutex__");
                        read(mutex);
                        run(format("echo %d > /ipc/__ipc_service_music_pid__", get_pid()));
                        run(format("window_playlist %s > /bat/%d", p2 + 1, get_pid()));
                        run(format("rm /ipc/__ipc_service_music_pid__"));
                        close(mutex);
                        exit(0);
                    }
                }
                else {
                    suc = 0;
                }
                if (suc == 1)
                    run(format("echo OK. > /ipc/res_%s", uuid));
                else
                    run(format("echo Failed. > /ipc/res_%s", uuid));
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
    shell("rm /fifo/ipc_service_music");
    shell("rm /ipc/service_music");
    shell("rm /mutex/__ipc_service_music_mutex__");
    put_string("waiting for process exit...\n");
    send_signal(get_pid(), 9);
    wait();
    put_string("service music stopped\n");
    return 0;
}