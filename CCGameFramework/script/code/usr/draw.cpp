#include "/include/shell"
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
int main(int argc, char **argv) {
    shell("/usr/draw_3dball"); waiting(2);
    shell("/usr/draw_heart1"); waiting(2);
    shell("/usr/draw_heart2"); waiting(2);
    shell("/usr/draw_heart3"); waiting(2);
    return 0;
}