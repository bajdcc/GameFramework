#include "/include/io"
#include "/include/window"
#include "/include/fs"
#include "/include/memory"
#include "/include/string"
#include "/include/shell"
#include "/include/xtoa_itoa"
#include "/include/sys"
#include "/include/gui"
int read_file(int id, int handle) {
    int c;
    window_layout_linear_set_vertical_align(window_get_base(id));
    window_set_style(id, style_win10_white);
    long text = window_create_comctl(id, comctl_label);
    long text2 = window_create_comctl(id, comctl_label);
    long text3 = window_create_comctl(id, comctl_label);
    long text4 = window_create_comctl(id, comctl_image);
    window_comctl_connect(window_get_base(id), text);
    window_comctl_connect(window_get_base(id), text2);
    window_comctl_connect(window_get_base(id), text3);
    window_comctl_connect(window_get_base(id), text4);
    window_comctl_set_text(text2, "请稍后");
    window_comctl_set_bound(text, 10, 10, 290, 50);
    window_comctl_set_bound(text2, 10, 10, 290, 30);
    window_comctl_set_bound(text3, 10, 10, 290, 30);
    window_comctl_set_bound(text4, 10, 10, 290, 50);
    window_comctl_label_set_horizontal_align_middle(text);
    window_comctl_label_set_horizontal_align_middle(text2);
    window_comctl_label_set_horizontal_align_middle(text3);
    window_comctl_image_set_ptr_from_url(text4, "/usr/loading.gif");
    __window_msg_struct__ s;
    if (fork() == -1) {
        int i;
        for (i = 0;; i++) {
            window_post_msg(id, 0x889, -1, i + 1, 0);
            sleep(1000);
            if (recv_signal() == 9) break;
        }
        sleep(500);
        window_post_msg(id, 0x888, -1, 0, 0);
        exit(0);
    }
    while (c = window_get_msg(handle, &s), c < 0x1000) {
        if (s.code == 0x888) {
            break;
        }
        if (s.code == 0x889) {
            char* str = malloc(100);
            char* fmt = malloc(20);
            i32toa(s.param1, fmt);
            strcpy(str, "* ");
            strcat(str, fmt);
            strcat(str, " *");
            window_comctl_set_text(text3, str);
            free(fmt);
            free(str);
        }
        else if (s.code == 0x111 && s.param1 == 1) {
            int left = gui_get_width() / 2 - 150;
            int top = gui_get_height() / 2 - 100;
            int width = 300;
            int height = 200;
            window_comctl_set_bound(window_get_base(id), left, top, left + width, top + height);
        }
        else if (s.code == 0x214 || s.code == 0x216 || s.code == 0x10) {

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
int main(int argc, char** argv) {
    __window_create_struct__ s;
    s.caption = "* 开机中 *";
    s.left = gui_get_width() / 2 - 150;
    s.top = gui_get_height() / 2 - 100;
    s.width = 300;
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
        read_file(id, handle);
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
    return 0;
}
