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
    shell("echo [*] 启动进程间通信主机！ > /fifo/sys_entry_console");
    shell("touch /fifo/ipc_service");
    shell("mklink /ipc/service /fifo/ipc_service hide");
    put_string("service started\n");
    char* data; int len;
    for (;;) {
        if (recv_signal() == 9) break;
        if (readfile("/ipc/service", &data, &len) != 0) {
            sleep(100);
            continue;
        }
        show(format("request come: %s\n", data));
        if (fork() == -1) {
            redirect_to_parent();
            char* uuid = data;
            char* path = format("/ipc/req_%s", uuid);
            show(format("reading: %s\n", path));
            int result;
            if ((result = readfile(path, &data, &len)) == 0) {
                char* t = data;
                char* p = strchr(data, ' ');
                show(format("read ok, content: %s\n", t));
                if (!p) {
                    p = " ";
                }
                *p = '\0';
                char* f = format("/ipc/service_%s", t );
                if (strcmp(t, "sys") == 0 || exists(f)) {
                    show(format("proxy %s %s\n", t, p + 1));
                    char* puuid; int plen;
                    if (readfile("/dev/uuid", &puuid, &plen) == 0) {
                        run(format("touch /fifo/ipcx_req_%s", puuid));
                        run(format("touch /fifo/ipcx_res_%s", puuid));
                        run(format("mklink /ipc/req_%s /fifo/ipcx_req_%s hide", puuid, puuid));
                        run(format("mklink /ipc/res_%s /fifo/ipcx_res_%s hide", puuid, puuid));
                        run(format("echo %s > /ipc/service_%s", puuid, t));
                        *p = ' ';
                        run(format("echo %s > /ipc/req_%s", t, puuid));
                        run(format("cat /ipc/res_%s > /ipc/res_%s", puuid, uuid));
                        run(format("rm /ipc/req_%s", puuid));
                        run(format("rm /ipc/res_%s", puuid));
                        run(format("rm /fifo/ipcx_req_%s", puuid));
                        run(format("rm /fifo/ipcx_res_%s", puuid));
                        run(format("rm /ipc/req_%s", uuid));
                        run(format("rm /ipc/res_%s", uuid));
                        run(format("rm /fifo/ipc_req_%s", uuid));
                        run(format("rm /fifo/ipc_res_%s", uuid));
                        free(puuid);
                    }
                }
                else {
                    run(format("echo Error: invalid call > /ipc/res_%s", uuid));
                    show(format("service not found: %s\n", t));
                }
                free(f);
                free(data);
            }
            else {
                run(format("echo Error: read error > /ipc/res_%s", uuid));
                show(format("read error: %d, %s\n", result, uuid));
            }
            show(format("request done: %s\n", uuid));
            free(uuid);
            free(path);
            exit(0);
        }
        show("fork new request\n");
        pipe();
        free(data);
    }
    shell("rm /fifo/ipc_service");
    shell("rm /ipc/service");
    put_string("service stopped\n");
    return 0;
}