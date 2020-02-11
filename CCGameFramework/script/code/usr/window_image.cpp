#include "/include/shell"
#include "/include/proc"
#include "/include/arg"
#include "/include/readfile"
#include "/include/xtoa_itoa"
#include "/include/window"
void show(char* path);
int read_file(int id, int handle, char* path);
int main(int argc, char** argv) {
    if (argc <= 1) {
        set_fg(240, 0, 0);
        put_string("[Error] Missing argument.");
        restore_fg();
        return 1;
    }
    char* out; int n;
    if (readfile("/dev/uuid", &out, &n) != 0) {
        set_fg(240, 0, 0);
        put_string("[Error] Get uuid failed.");
        restore_fg();
        return 2;
    }
    char* path = arg_string(1, argc, argv);
    char* buf = malloc(256);
    strcpy(buf, "echo ");
    strcat(buf, path);
    strcat(buf, " | copy ");
    strcat(buf, "/tmp/window-image-");
    strcat(buf, out);
    strcat(buf, ".image");
    put_string("# ");
    put_string(buf);
    put_string("\n");
    shell(buf);
    strcpy(buf, "/tmp/window-image-");
    strcat(buf, out);
    strcat(buf, ".image");
    int ff = fsize(buf);
    if (ff <= 0) {
        if (ff == 0) rm(buf);
        set_fg(240, 0, 0);
        put_string("[Error] File not exists.");
        restore_fg();
        return 3;
    }
    show(buf);
    rm(buf);
    return 0;
}

void show(char* path) {
    __window_create_struct__ s;
    s.caption = "图片预览";
    s.left = 50;
    s.top = 50;
    s.width = 360;
    s.height = 360;
    int id = window_create(&s);
    put_string("Create image window: ");
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
        read_file(id, handle, path);
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
}

int read_file(int id, int handle, char* path) {
    int c;
    window_layout_linear_set_vertical_align(window_get_base(id));
    window_set_style(id, style_win10_white);
    long image = window_create_comctl(id, comctl_image);
    window_comctl_connect(window_get_base(id), image);
    window_comctl_set_bound(image, 10, 10, 350, 320);
    window_comctl_image_set_ptr_from_url(image, path);
    __window_msg_struct__ s;
    while (c = window_get_msg(handle, &s), c < 0x1000) {
        if (s.code == 0x214) {

        }
        else if (s.code < 0x800) {
            window_default_msg(id, &s);
        }
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
    close(id);
}