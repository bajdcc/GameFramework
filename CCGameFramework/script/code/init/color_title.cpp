#include "/include/gui"
int main(int argc, char **argv) {
    gui_lua("CurrentScene.layers.text.color = '#0000F0'; CurrentScene.layers.text:update_and_paint();");
    return 0;
}