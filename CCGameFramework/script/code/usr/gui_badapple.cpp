#include "/include/proc"
#include "/include/io"
#include "/include/gui"
#include "/include/fs"
#include "/include/sys"
int hex2char(int n) {
    if (n >= (int) '0' && n <= (int) '9')
        return n - (int) '0';
    if (n >= (int) 'A' && n <= (int) 'F')
        return 10 + n - (int) 'A';
    return 0;
}
int x = 0, y = 0;
int read_file(int handle) {
    int W = 80, H = 25;
    int scale_w = 7, scale_h = 12;
    int width = gui_width(), height = gui_height();
    int start_x = (width - W * scale_w) / 2;
    int start_y = (height - H * scale_h) / 2;

    gui_power_on();
    int c, flag = 0, num, pixels = W * H, px = 0, j;
    int p;
    set_cycle(1000000);
    sleep(0);
    gui_clear(0, 0, 0);
    gui_disable_fresh();
    while (c = read(handle), c >= 0) {
        if (c == (int) ' ' || c == (int) '\r' || c == (int) '\n') { // skip
            continue;
        }
        if (flag == 0) {
            num = hex2char(c);
            flag = 1;
        }
        else {
            num <<= 4;
            num += hex2char(c);
            flag = 0;
            if ((num & 128) != 0) {
                num -= 128;
                gui_rgb(255, 255, 255);
                p = 1;
            }
            else {
                gui_rgb(0, 0, 0);
                p = 0;
            }
            if (px + num > pixels) {
                if (p == 1) {
                    for (j = 0; j < pixels - px; j++) {
                        gui_move_to(start_x + ((px + j) % 80) * scale_w, start_y + ((px + j) / 80) * scale_h);
                        gui_rect(start_x + ((px + j) % 80) * scale_w + scale_w - 1, start_y + ((px + j) / 80) * scale_h + scale_h - 1);
                    }
                }
                px += num - pixels;
                gui_fresh();
                sleep(-33);
                gui_clear(0, 0, 0);
                x = 0; y = 0;
                if (p == 1) {
                    for (j = 0; j < px; j++) {
                        gui_move_to(start_x + ((j) % 80) * scale_w, start_y + ((j) / 80) * scale_h);
                        gui_rect(start_x + ((j) % 80) * scale_w + scale_w - 1, start_y + ((j) / 80) * scale_h + scale_h - 1);
                    }
                }
            }
            else {
                if (p == 1) {
                    for (j = 0; j < num; j++) {
                        gui_move_to(start_x + ((px + j) % 80) * scale_w, start_y + ((px + j) / 80) * scale_h);
                        gui_rect(start_x + ((px + j) % 80) * scale_w + scale_w - 1, start_y + ((px + j) / 80) * scale_h + scale_h - 1);
                    }
                }
                px += num;
            }
        }
    }
    gui_enable_fresh();
    gui_power_off();
    switch (c) {
    case -1:
        // put_string("[INFO] Read to the end.\n");
        put_string("\n");
        break;
    case -2:
        set_fg(240, 0, 0);
        put_string("[ERROR] File already deleted.\n");
        restore_fg();
        break;
    case -3:
        set_fg(240, 0, 0);
        put_string("[ERROR] Invalid handle.\n");
        restore_fg();
        break;
    }
    close(handle);
}
int main(int argc, char** argv) {
    int handle = open("/usr/badapple.txt");
    switch (handle) {
    default:
        // put_string("[INFO] Success.\n");
        read_file(handle);
        break;
    case -1:
        set_fg(240, 0, 0);
        put_string("[ERROR] File not exists.\n");
        restore_fg();
        break;
    case -2:
        set_fg(240, 0, 0);
        put_string("[ERROR] Path is not file.\n");
        restore_fg();
        break;
    case -3:
        set_fg(240, 0, 0);
        put_string("[ERROR] File is locked.\n");
        restore_fg();
        break;
    }
    return 0;
}