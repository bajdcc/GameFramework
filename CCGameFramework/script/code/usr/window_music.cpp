#include "/include/io"
#include "/include/window"
#include "/include/fs"
#include "/include/memory"
#include "/include/string"
#include "/include/xtoa_itoa"
#include "/include/proc"
#include "/include/readfile"
#include "/include/writefile"
#include "/include/json"
#include "/include/shell"
#include "/include/sys"
#include "/include/format"
void play(char *name, int id);
char* song_names[0] = {
    "摩天动物园",
    "透明",
    "Take me hand",
    "まっしろな雪",
    "Faded",
    "Nevada",
    "New world",
    "Closer",
    "Intro",
    "Journey",
    "Ferrari",
};
int song_id[0] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int child = -1;
long text, text2, text3, text4;
void pipe() {
    int c;
    while ((c = io_pipe()) != -1) {
        put_char((char)c);
    }
}
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
    window_comctl_set_text(text, "播放");
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
        if (recv_signal() == 9) break;
        pipe();
        if (s.code == 0x201 || s.code == 0x888) {
            if (s.comctl == t1id || s.code == 0x888) {
                if (child != -1) {
                    newline();
                    send_signal(child, 9);
                    wait();
                    child = -1;
                }
                if ((child = fork()) == -1) {
                    play(song_names[i], song_id[i]);
                    if (recv_signal() == 9) exit(1);
                    s.code = 0x888;
                    s.comctl = -1;
                    window_default_msg(id, &s);
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
    send_signal(child, 9);
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
    s.caption = "在线听歌";
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
    char* path = format("/http/post!music.163.com/api/search/suggest/web!s=%s", name);
    put_string("Open: "); put_string(path); put_string("\n");
    char* json; int len;
    if (readfile(path, &json, &len) == 0) {
        json_object* obj = json_parse_obj(json);
        if (obj) {
            put_string("Code: ");
            int code = json_obj_get_string(obj, "code")->data.i;
            put_int(code);
            put_string("\n");
            if (code != 200) goto FAILED;
            json_object* result = json_obj_get_string(obj, "result");
            if (!result) goto FAILED;
            json_object* songs = json_obj_get_string(result, "songs");
            put_string("Count of songs: ");
            int N = json_array_size(songs);
            put_int(N);
            put_string("\n");
            int i;
            if (N == 0) goto FAILED;
            for (i = 0; i < N; i++) {
                put_string(" --> ["); put_int(i); put_string("] ");
                json_object* s = json_array_get(songs, i);
                put_string("id= ");
                put_int(json_obj_get_string(s, "id")->data.i);
                put_string(", name= ");
                put_string(json_obj_get_string(s, "name")->data.str);
                put_string("\n");
            }
            put_string("Choose: ");
            put_int(id);
            put_string("\n");
            json_object* s = json_array_get(songs, id);
            char* downurl = malloc(200);
            int sid = json_obj_get_string(s, "id")->data.i; 
            window_comctl_set_text(text2, json_obj_get_string(s, "name")->data.str);
            window_comctl_set_text(text3, json_obj_get_string(json_array_get(json_obj_get_string(s, "artists"), 0), "name")->data.str);
            char* picurl = json_obj_get_string(json_obj_get_string(json_obj_get_string(s, "album"), "artist"), "img1v1Url")->data.str;
            int picurl_free = 0;
            char* tmp = malloc(20);
            i32toa(sid, tmp);
            {
                free(path);
                path = format("/http/post!music.163.com/api/song/detail!id=%s&ids=[%s]", tmp, tmp);
                put_string("Open: "); put_string(path); put_string("\n");
                char* json2; int len2;
                if (readfile(path, &json2, &len2) == 0) {
                    json_object* obj2 = json_parse_obj(json2);
                    if (obj2) {
                        char* pic = json_obj_get_string(json_obj_get_string(json_array_get(json_obj_get_string(obj2, "songs"), 0), "album"), "picUrl")->data.str;
                        put_string("Image: "); put_string(pic); put_string("\n");
                        picurl_free = 1;
                        picurl = malloc(strlen(pic) + 1);
                        strcpy(picurl, pic);
                        free(obj2);
                    }
                    free(json2);
                }
            }
            // MP3
            strcpy(downurl, "/tmp/");
            strcat(downurl, tmp);
            strcat(downurl, ".mp3");
            int empty = fsize(downurl);
            if (empty <= 0) {
                if (empty == 1) rm(downurl);
                put_string("Saved music to ");
                put_string(downurl);
                put_string("\n");
                free(downurl);
                downurl = format("echo /http/bin!music.163.com/song/media/outer/url?id=%s.mp3 | copy /tmp/%s.mp3", tmp, tmp);
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
            // LYRIC
            strcpy(downurl, "/tmp/");
            strcat(downurl, tmp);
            strcat(downurl, ".txt");
            empty = fsize(downurl);
            if (empty <= 0) {
                if (empty == 1) rm(downurl);
                put_string("Saved lyric to ");
                put_string(downurl);
                put_string("\n");
                free(downurl);
                downurl = format("echo /http/music.163.com/api/song/media?id=%s | copy /tmp/%s.txt", tmp, tmp);
                put_string("# ");
                put_string(downurl);
                put_string("\n");
                shell(downurl);
                put_string("Download OK\n");
                char* lyric_path = format("/tmp/%s.txt", tmp);
                char* lyric_txt; int lryic_len;
                if (readfile(lyric_path, &lyric_txt, &lryic_len) == 0) {
                    json_object* lyric_obj = json_parse_obj(lyric_txt);
                    if (lyric_obj) {
                        put_string("Code: "); put_int(json_obj_get_string(lyric_obj, "code")->data.i); put_string("\n");
                        char* lyric = json_obj_get_string(lyric_obj, "lyric")->data.str;
                        writefile(lyric_path, lyric, strlen(lyric), 1);
                    }
                    free(lyric_txt);
                }
                free(lyric_path);
            }
            else {
                put_string("OK, ");
                put_string(downurl);
                put_string(" exists\n");
            }
            // PIC
            strcpy(downurl, "/tmp/");
            strcat(downurl, tmp);
            strcat(downurl, ".jpg");
            empty = fsize(downurl);
            if (empty <= 0) {
                if (empty == 0) rm(downurl);
                put_string("Saved pic to ");
                put_string(downurl);
                put_string("\n");
                free(downurl);
                downurl = format("echo /http/bin!%s | copy /tmp/%s.jpg", picurl + 7, tmp);
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
            free(obj);
            // PLAY
            put_string("Setting pic\n");
            strcpy(downurl, "/tmp/");
            strcat(downurl, tmp);
            strcat(downurl, ".jpg");
            window_comctl_image_set_ptr_from_url(text4, downurl);
            put_string("Playing\n");
            strcpy(downurl, "cat /music/tmp/");
            strcat(downurl, tmp);
            strcat(downurl, ".mp3!/tmp/");
            strcat(downurl, tmp);
            strcat(downurl, ".txt");
            put_string("# ");
            put_string(downurl);
            put_string("\n");
            shell(downurl);
            newline();
            put_string("Play OK\n");
            free(tmp);
            free(downurl);
            if (picurl_free == 1) free(picurl);
        FAILED:
            ;
        }
        else {
            put_string("Parsing json failed.\n");
        }
        free(json);
    }
    else {
        put_string("Open failed.\n");
    }
    free(path);
}