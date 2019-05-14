#include "/include/io"
#include "/include/gui"
#include "/include/proc"
int waiting(int second) {
    while (second > 0) {
        put_string("Waiting... ");
        put_int(second--);
        put_string("s");
        put_string("\r");
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
int main(int argc, char **argv) {
    put_string("========== [#9 TEST GUI] ==========\n");
    power_on();
    waiting(4);
    power_off();
    put_string("========== [#9 TEST GUI] ==========\n");
    return 0;
}