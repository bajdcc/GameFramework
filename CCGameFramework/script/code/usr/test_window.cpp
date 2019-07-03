#include "/include/io"
#include "/include/window"
#include "/include/fs"
#include "/include/memory"
#include "/include/string"
#include "/include/xtoa_itoa"
int read_file(int handle) {
    int c;
    while (c = read(handle), c < 0x1000) {
    }
    switch (c) {
    case 0x2000:
        // put_string("[INFO] Read to the end.");
        break;
    case 0x2001:
        set_fg(240, 0, 0);
        put_string("[ERROR] Read error.");
        restore_fg();
        break;
    }
    close(handle);
}
int main(int argc, char **argv) {
    put_string("========== [#15 TEST WINDOW] ==========\n");
    __window_create_struct__ s;
    s.caption = "Test window";
    s.left = 10;
    s.top = 50;
    s.width = 200;
    s.height = 200;
    int id = window_create(&s);
    put_string("Create test window: ");
    put_hex(id);
    put_string("\n");
    char* msg = malloc(20);
    strcpy(msg, "/handle/");
    char* fmt = malloc(20);
    i32toa(id, fmt);
    strcat(msg, fmt);
    strcat(msg, "/message");
    int handle = open(msg);
    switch (handle) {
        default:
            // put_string("[INFO] Success.");
            read_file(handle);
            break;
        case -1:
            set_fg(240, 0, 0);
            put_string("[ERROR] File not exists.");
            restore_fg();
            break;
        case -2:
            set_fg(240, 0, 0);
            put_string("[ERROR] Path is not file.");
            restore_fg();
            break;
        case -3:
            set_fg(240, 0, 0);
            put_string("[ERROR] File is locked.");
            restore_fg();
            break;
    }
    //window_wait(id);
    put_string("========== [#15 TEST WINDOW] ==========\n");
    return 0;
}