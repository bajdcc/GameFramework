#include "/include/io"
#include "/include/window"
int main(int argc, char **argv) {
    put_string("========== [#15 TEST WINDOW] ==========\n");
    __window_create_struct__ s;
    s.caption = "Test window";
    s.left = 10;
    s.top = 10;
    s.width = 200;
    s.height = 200;
    int id = window_create(&s);
    put_string("Create test window: ");
    put_hex(id);
    put_string("\n");
    window_wait(id);
    put_string("========== [#15 TEST WINDOW] ==========\n");
    return 0;
}