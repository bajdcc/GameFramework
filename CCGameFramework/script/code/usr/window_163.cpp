#include "/include/window"
#include "/include/fs"
#include "/include/string"
#include "/include/proc"
#include "/include/readfile"
#include "/include/json"
#include "/include/shell"
#include "/include/sys"
#include "/include/format"
#include "/include/gui"
int child = -1;
long text, text2, text3, text4, text5;
char* list64 = "NjA3NzEwMjk=";
void play(char* id);
char* down_playlist(char* id);
int read_file(int id, int handle, char* playlist) {
    char* begin = strstr(playlist, "track_playlist");
    if (begin == (char*)0) {
        set_fg(240, 0, 0);
        put_string("[ERROR] Playlist find begin error.");
        restore_fg();
        exit(0);
    }
    put_string("Begin: "); put_hex(begin); put_string("\n");
    char* end = strstr(begin, "song-list-pre-data");
    if (end == (char*)0) {
        set_fg(240, 0, 0);
        put_string("[ERROR] Playlist find end error.");
        restore_fg();
        exit(0);
    }
    put_string("End: "); put_hex(end); put_string("\n");
    int c;
    window_layout_linear_set_vertical_align(window_get_base(id));
    window_set_style(id, style_win10_white);
    text = window_create_comctl(id, comctl_button);
    text2 = window_create_comctl(id, comctl_label);
    text3 = window_create_comctl(id, comctl_label);
    text4 = window_create_comctl(id, comctl_image);
    text5 = window_create_comctl(id, comctl_button);
    window_comctl_connect(window_get_base(id), text);
    window_comctl_connect(window_get_base(id), text5);
    window_comctl_connect(window_get_base(id), text2);
    window_comctl_connect(window_get_base(id), text3);
    window_comctl_connect(window_get_base(id), text4);
    window_comctl_set_text(text, "播放");
    window_comctl_set_text(text2, "");
    window_comctl_set_text(text3, "");
    window_comctl_set_text(text5, "控制");
    window_comctl_set_bound(text, 10, 10, 200, 30);
    window_comctl_set_bound(text2, 10, 10, 200, 30);
    window_comctl_set_bound(text3, 10, 10, 200, 30);
    window_comctl_set_bound(text4, 10, 10, 200, 200);
    window_comctl_set_bound(text5, 10, 10, 200, 30);
    window_comctl_label_set_horizontal_align_middle(text);
    window_comctl_label_set_horizontal_align_middle(text2);
    window_comctl_label_set_horizontal_align_middle(text3);
    window_comctl_label_set_horizontal_align_middle(text5);
    int t1id = window_get_comctl(text);
    int t5id = window_get_comctl(text5);
    __window_msg_struct__ s;
    int i = 0;
    char* ptr = begin;
    char* ids = malloc(32);
    *ids = '\0';
    while (c = window_get_msg(handle, &s), c < 0x1000) {
        if (s.code == 0x201 || s.code == 0x888) {
            if (s.comctl == t1id || s.code == 0x888) {
                if (child != -1) {
                    newline();
                    send_signal(child, 99);
                    child = -1;
                    sleep(200);
                }
                window_comctl_set_text(text5, "控制");
                if (*ids != '\0') {
                    char* downurl = format("rm /tmp/%s.mp3", ids);
                    put_string(downurl);
                    put_string("\n");
                    shell(downurl);
                    free(downurl);
                    downurl = format("rm /tmp/%s.jpg", ids);
                    put_string(downurl);
                    put_string("\n");
                    shell(downurl);
                    free(downurl);
                }
                put_string("Searching...\n");
                char* ptr2 = strstr(ptr, "href=\"/song?id=");
                *ids = '\0';
                if (ptr2) {
                    put_string("Ptr2: "); put_hex(ptr2); put_string("\n");
                    char* ptr3 = strstr(ptr2 + 15, "\"");
                    if (ptr3) {
                        put_string("Ptr3: "); put_hex(ptr3); put_string("\n");
                        int len = ((int)(ptr3 - ptr2)) - 15;
                        strncpy(ids, ptr2 + 15, len);
                        ids[len] = '\0';
                        put_string("Id: "); put_string(ids); put_string("\n");
                        ptr = ptr3;
                    }
                }
                if (*ids == '\0') {
                    ptr = begin;
                    s.code = 0x888;
                    s.comctl = -1;
                    window_default_msg(id, &s);
                }
                else if ((child = fork()) == -1) {
                    play(ids);
                    s.code = 0x888;
                    s.comctl = -1;
                    window_default_msg(id, &s);
                    exit(0);
                }
            }
            else if (s.comctl == t5id) {
                int n = gui_music_ctrl(0);
                switch (n) {
                case 0:
                    window_comctl_set_text(text5, "状态：无音乐");
                    break;
                case 1:
                    window_comctl_set_text(text5, "状态：播放中");
                    break;
                case 2:
                    window_comctl_set_text(text5, "状态：暂停");
                    break;
                case 3:
                    window_comctl_set_text(text5, "状态：播放中");
                    break;
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
int main(int argc, char** argv) {
    char* list; int listL;
    char* sh = format("echo %s | base64_decode > /tmp/163_list", list64);
    shell(sh);
    free(sh);
    if (readfile("/tmp/163_list", &list, &listL) != 0) {
        return -1;
    }
    char* playlist = down_playlist(list);
    if (playlist == (char*)0) {
        set_fg(240, 0, 0);
        put_string("[ERROR] Download playlist failed.");
        restore_fg();
        exit(0);
    }
    __window_create_struct__ s;
    s.caption = "在线听歌";
    s.left = 50;
    s.top = 50;
    s.width = 210;
    s.height = 320;
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
        read_file(id, handle, playlist);
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

void play(char* id) {
    newline(); put_string("Id: "); put_string(id); put_string("\n");
    char* path = format("/http/post!music.163.com/api/song/detail!id=%s&ids=[%s]", id, id);
    put_string("Open: "); put_string(path); put_string("\n");
    char* json = readfile_fast(path);
    if (json != (char*)0) {
        json_object* obj = json_parse_obj(json);
        if (obj) {
            put_string("Code: "); put_int(json_obj_get_string(obj, "code")->data.i); put_string("\n");
            // Name
            char* name = json_obj_get_string(json_array_get(json_obj_get_string(obj, "songs"), 0), "name")->data.str;
            put_string("Name: "); put_string(name); put_string("\n");
            window_comctl_set_text(text2, name);
            // Artist
            char* artist = json_obj_get_string(json_array_get(json_obj_get_string(json_array_get(json_obj_get_string(obj, "songs"), 0), "artists"), 0), "name")->data.str;
            put_string("Artist: "); put_string(artist); put_string("\n");
            window_comctl_set_text(text3, artist);
            // Image
            char* pic = json_obj_get_string(json_obj_get_string(json_array_get(json_obj_get_string(obj, "songs"), 0), "album"), "picUrl")->data.str;
            put_string("Image: "); put_string(pic); put_string("\n");
            char* picurl = malloc(strlen(pic) + 1);
            strcpy(picurl, pic);
            free(obj);
            // MP3
            char* downurl = format("/tmp/%s.mp3", id);
            int empty = fempty(downurl);
            if (empty < 0 || empty == 1) {
                if (empty == 1) rm(downurl);
                put_string("Saved music to ");
                put_string(downurl);
                put_string("\n");
                free(downurl);
                downurl = format("echo /http/bin!music.163.com/song/media/outer/url?id=%s.mp3 | copy /tmp/%s.mp3", id, id);
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
            // PIC
            free(downurl);
            downurl = format("/tmp/%s.jpg", id);
            empty = fempty(downurl);
            if (empty < 0 || empty == 1) {
                if (empty == 1) rm(downurl);
                put_string("Saved pic to ");
                put_string(downurl);
                put_string("\n");
                free(downurl);
                downurl = format("echo /http/bin!%s | copy /tmp/%s.jpg", picurl + 7, id);
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
            // PLAY
            put_string("Setting pic\n");
            free(downurl);
            downurl = format("/tmp/%s.jpg", id);
            window_comctl_image_set_ptr_from_url(text4, downurl);
            put_string("Playing\n");
            free(downurl);
            downurl = format("cat /music/tmp/%s.mp3", id);
            put_string("# ");
            put_string(downurl);
            put_string("\n");
            shell(downurl);
            newline();
            put_string("Play OK\n");
            free(downurl);
            free(picurl);
        }
        free(json);
    }
}
char* down_playlist(char* id) {
    char* path = format("/http/music.163.com/playlist?id=%s", id);
    newline(); put_string("Open: "); put_string(path); put_string("\n");
    char* json; int len;
    if (readfile(path, &json, &len) == 0) {
        return json;
    }
    return (char*)0;
}