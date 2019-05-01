#include "/include/proc"
#include "/include/io"
#include "/include/fs"
#include "/include/sys"
int hex2char(int n) {
    if (n >= (int) '0' && n <= (int) '9')
        return n - (int) '0';
    if (n >= (int) 'A' && n <= (int) 'F')
        return 10 + n - (int) 'A';
    return 0;
}
int frame, last_frame;
long time;
float fps;
int print_fps() {
    long now = timestamp();
    if (now - time > 1000000) { // 间隔一秒
        if (now - time > 3000000) { // 间隔三秒以上，重置计时
            sleep(0);
        }
        if (frame > last_frame) {
            fps = 1000000.0f * (float) (frame - last_frame) / (float) (now - time);
        }
        time = now;
        last_frame = frame;
    }
    set_bg(125,130,130);
    put_string("[INFO] FPS: ");
    put_float(fps);
    put_string(" TIME: ");
    put_long(time);
    restore_bg();
    put_string("\n");
}
int print_frame() {
    set_bg(75, 77, 77);
    put_string("@clibos                   BADAPPLE ASCII-ART ANIMATION              Frame: ");
    put_int(frame);
    restore_bg();
    put_string("\n");
}
int read_file(int handle) {
    int c, flag = 0, num, pixels = 80 * 25, px = 0, j;
    char p;
    fps = 0;
    frame = last_frame = 1;
    time = timestamp();
    resize(30, 80);
    put_char('\f');
    print_frame();
    set_cycle(1000000);
    sleep(0);
    while (c = read(handle), c >= 0) {
        if (c == (int) ' ' || c == (int) '\r' || c == (int) '\n') { // skip
            continue;
        }
        if (flag == 0) {
            num = hex2char(c);
            flag = 1;
        } else {
            num <<= 4;
            num += hex2char(c);
            flag = 0;
            if ((num & 128) != 0) {
                num -= 128;
                p = '\7';
            } else {
                p = ' ';
            }
            if (px + num > pixels) {
                for (j = 0; j < pixels - px; j++) {
                    put_char(p);
                }
                px += num - pixels;
                print_frame();
                print_fps();
                sleep(-33);
                frame++;
                put_char('\f');
                print_frame();
                for (j = 0; j < px; j++) {
                    put_char(p);
                }
            } else {
                px += num;
                for (j = 0; j < num; j++) {
                    put_char(p);
                }
            }
        }
    }
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
int main(int argc, char **argv) {
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