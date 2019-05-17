#include "/include/io"
#include "/include/gui"
#include "/include/proc"
#include "/include/shell"
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
int main(int argc, char **argv) {
    put_string("========== [#9 TEST GUI] ==========\n");
    gui_power_on();
    waiting(1);
    shell("/usr/gui_3dball"); waiting(2);
    shell("/usr/gui_heart"); waiting(2);
    gui_power_off();
    put_string("========== [#9 TEST GUI] ==========\n");
    return 0;
}