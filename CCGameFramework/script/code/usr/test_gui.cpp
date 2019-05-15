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
void power_on() {
    gui_enable(1);
}
void power_off() {
    gui_enable(0);
}
void draw_1() {
    gui_move_to(10, 10);
    gui_line_to(10, 500);
    gui_line_to(500, 500);
    gui_line_to(500, 10);
    gui_line_to(10, 10);
}
int main(int argc, char **argv) {
    put_string("========== [#9 TEST GUI] ==========\n");
    power_on();
    waiting(1);
    draw_1();
    waiting(5);
    power_off();
    put_string("========== [#9 TEST GUI] ==========\n");
    return 0;
}