#include "/include/io"
#include "/include/gui"
#include "/include/proc"
int waiting(int second) {
    while (second > 0) {
        put_string("Waiting... ");
        put_int(second--);
        put_string("s");
        put_string("    \r");
        sleep(1000);
    }
    put_string("\n");
}
void draw_1() {
    int width = gui_width(), height = gui_height();
    gui_move_to(10, 10);
    gui_line_to(10, width - 10);
    gui_line_to(height - 10, width - 10);
    gui_line_to(height - 10, 10);
    gui_line_to(10, 10);
}
int main(int argc, char **argv) {
    put_string("========== [#9 TEST GUI] ==========\n");
    gui_power_on();
    waiting(1);
    draw_1();
    waiting(5);
    gui_power_off();
    put_string("========== [#9 TEST GUI] ==========\n");
    return 0;
}