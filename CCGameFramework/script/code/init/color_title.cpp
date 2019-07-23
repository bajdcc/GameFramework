#include "/include/gui"
#include "/include/memory"
#include "/include/string"
#include "/include/math"
#include "/include/shell"
#include "/include/proc"
void set_title(char* s, char* color) {
    strcpy(s, "CurrentScene.layers.text.color = '#");
    strcat(s, color);
    strcat(s, "'; CurrentScene.layers.text:update_and_paint();");
    gui_lua(s);
}
char* map_hex = "0123456789ABCDEF";
char* get_hex(char *s, int r) {
    int i = 0, j;
    while (r) {
        s[i++] = map_hex[r % 16];
        r /= 16;
    }
    s[i--] = '\0';
    for (j = 0; j < i / 2; j++) {
        char t = s[j];
        s[j] = s[i - j];
        s[i - j] = t;
    }
    return s;
}
int main(int argc, char** argv) {
    char* s = malloc(200);
    char* c = malloc(10);
    int i;
    for (i = 0; i < 100; i++) {
        int color = gui_hsl2rgb(rand(240), 240, 120);
        strcpy(c, get_hex("AAAAAAA", color));
        set_title(s, c);
        if (recv_signal() == 9) break;
        shell("sleep 5");
    }
    return 0;
}