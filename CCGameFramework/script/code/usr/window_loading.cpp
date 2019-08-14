#include "/include/io"
#include "/include/window"
#include "/include/fs"
#include "/include/memory"
#include "/include/string"
#include "/include/xtoa_itoa"
#include "/include/proc"
#include "/include/shell"
#include "/include/sys"
void play(char *name, int id);
char* song_names[0] = {
    "www.sucaijishi.com/uploadfile/2015/0210/20150210104950809.gif",
    "www.sucaijishi.com/uploadfile/2015/0210/20150210104951492.gif",
    "www.sucaijishi.com/uploadfile/2015/0210/20150210104951657.gif",
    "www.sucaijishi.com/uploadfile/2015/0210/20150210104951663.gif",
    "www.sucaijishi.com/uploadfile/2015/0210/20150210104952191.gif",
    "www.sucaijishi.com/uploadfile/2015/0210/20150210104952890.gif",
    "www.sucaijishi.com/uploadfile/2015/0210/20150210104952902.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034914306.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034914143.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034914902.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034914443.gif",
    "www.sucaijishi.com/uploadfile/2014/0605/20140605015940688.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034914229.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034915500.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034915795.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034915742.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034915864.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034916110.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034916434.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034916363.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034916454.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034917708.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034917759.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034917143.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034917946.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034918729.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034918372.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034918260.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034918234.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034919654.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034919253.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034919595.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034920684.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034920497.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034920629.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034921405.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034921258.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034921885.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034922242.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034922697.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034922470.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034922191.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034923804.gif",
    "www.sucaijishi.com/uploadfile/2013/0527/20130527034923893.gif",
    "www.sucaijishi.com/uploadfile/2014/0524/20140524124233131.gif",
    "www.sucaijishi.com/uploadfile/2014/0524/20140524124234562.gif",
    "www.sucaijishi.com/uploadfile/2014/0524/20140524124237518.gif",
    "www.sucaijishi.com/uploadfile/2014/0524/20140524124238403.gif",
    "www.sucaijishi.com/uploadfile/2014/0524/20140524124239682.gif",
    "www.sucaijishi.com/uploadfile/2014/0524/20140524124240538.gif",
    "www.sucaijishi.com/uploadfile/2014/0524/20140524124241131.gif",
    "www.sucaijishi.com/uploadfile/2014/0524/20140524124241879.gif",
    "www.sucaijishi.com/uploadfile/2014/0524/20140524124245801.gif",
    "www.sucaijishi.com/uploadfile/2014/0524/20140524124246402.gif",
};
int child = -1;
long text, text2, text3, text4;
int read_file(int id, int handle) {
    int c;
    window_layout_linear_set_vertical_align(window_get_base(id));
    window_set_style(id, style_win10_white);
    text = window_create_comctl(id, comctl_button);
    text2 = window_create_comctl(id, comctl_label);
    text3 = window_create_comctl(id, comctl_label);
    text4 = window_create_comctl(id, comctl_image);
    window_comctl_connect(window_get_base(id), text);
    window_comctl_connect(window_get_base(id), text2);
    window_comctl_connect(window_get_base(id), text3);
    window_comctl_connect(window_get_base(id), text4);
    window_comctl_set_text(text, "预览");
    window_comctl_set_text(text2, "");
    window_comctl_set_text(text3, "");
    window_comctl_set_bound(text, 10, 10, 200, 30);
    window_comctl_set_bound(text2, 10, 10, 200, 30);
    window_comctl_set_bound(text3, 10, 10, 200, 30);
    window_comctl_set_bound(text4, 10, 10, 200, 200);
    window_comctl_label_set_horizontal_align_middle(text);
    window_comctl_label_set_horizontal_align_middle(text2);
    window_comctl_label_set_horizontal_align_middle(text3);
    int t1id = window_get_comctl(text);
    __window_msg_struct__ s;
    int i = 0;
    while (c = window_get_msg(handle, &s), c < 0x1000) {
        if (s.code == 0x201) {
            if (s.comctl == t1id) {
                if (child != -1) {
                    newline();
                    send_signal(child, 99);
                    child = -1;
                }
                if ((child = fork()) == -1) {
                    play(song_names[i], i + 1);
                    exit(0);
                }
                else {
                    i++;
                    if (i >= sizeof(song_names) / sizeof(int*)) i = 0;
                }
            }
        }
        else if (s.code == 0x214) {

        }
        else if (s.code < 0x800) {
            window_default_msg(id, &s);
        }
    }
    send_signal(child, 99);
    switch (c) {
    case 0x2000:
        // put_string("[INFO] Read to the end.");
        break;
    case 0x2001:
        set_fg(240, 0, 0);
        put_string("[ERROR] Read error.");
        restore_fg();
        break;
    }
    close(handle);
    close(id);
}
int main(int argc, char **argv) {
    __window_create_struct__ s;
    s.caption = "加载动画示例";
    s.left = 50;
    s.top = 50;
    s.width = 210;
    s.height = 310;
    int id = window_create(&s);
    put_string("Create test window: ");
    put_hex(id);
    put_string("\n");
    char* msg = malloc(20);
    strcpy(msg, "/handle/");
    char* fmt = malloc(20);
    i32toa(id, fmt);
    strcat(msg, fmt);
    strcat(msg, "/message");
    int handle = open(msg);
    switch (handle) {
        default:
            // put_string("[INFO] Success.");
            read_file(id, handle);
            break;
        case -1:
            set_fg(240, 0, 0);
            put_string("[ERROR] File not exists.");
            restore_fg();
            break;
        case -2:
            set_fg(240, 0, 0);
            put_string("[ERROR] Path is not file.");
            restore_fg();
            break;
        case -3:
            set_fg(240, 0, 0);
            put_string("[ERROR] File is locked.");
            restore_fg();
            break;
    }
    //window_wait(id);
    return 0;
}

void play(char* name, int id) {
    char* tmp = malloc(20);
    i32toa(id, tmp);
    put_string("Download gif from: ");
    put_string(name);
    put_string("\n");
    char* downurl = malloc(200);
    strcpy(downurl, "/tmp/loading_");
    strcat(downurl, tmp);
    strcat(downurl, ".gif");
    int empty = fempty(downurl);
    if (empty < 0 || empty == 1) {
        strcpy(downurl, "echo /http/bin!");
        strcat(downurl, name);
        strcat(downurl, " | copy /tmp/loading_");
        strcat(downurl, tmp);
        strcat(downurl, ".gif");
        put_string("# ");
        put_string(downurl);
        put_string("\n");
        shell(downurl);
        put_string("Download OK\n");
    }
    else {
        put_string("OK, ");
        put_string(downurl);
        put_string(" exists\n");
    }
    // SETTING
    put_string("Setting gif\n");
    strcpy(downurl, "/tmp/loading_");
    strcat(downurl, tmp);
    strcat(downurl, ".gif");
    window_comctl_set_text(text2, tmp);
    window_comctl_image_set_ptr_from_url(text4, downurl);
    put_string("Setting OK\n");
    free(downurl);
    free(tmp);
}