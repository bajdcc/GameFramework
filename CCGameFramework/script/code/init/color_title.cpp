#include "/include/gui"
#include "/include/memory"
#include "/include/string"
void set_title(char* s, char* color) {
    strcpy(s, "CurrentScene.layers.text.color = '#");
    strcat(s, color);
    strcat(s, "'; CurrentScene.layers.text:update_and_paint();");
    gui_lua(s);
}
int main(int argc, char** argv) {
    char* s = malloc(200);
    set_title(s, "0000F0");
    return 0;
}