#include "/include/fs"
#include "/include/string"
#include "/include/proc"
#include "/include/readfile"
#include "/include/json"
#include "/include/shell"
#include "/include/sys"
#include "/include/format"
#include "/include/xtoa_atoi"
int child = -1;
long text, text2, text3, text4, text5;
char* list64_name[0] = {
    "我的歌单",
    "葫芦丝2006",
    "李荣浩",
    "萨克斯",
    "东方",
    "Noicybino",
};
char* list64[0] = {
    "NjA3NzEwMjk=",
    "MzA4Mzc2MzMzNQ==",
    "Mjg5NjA1Njk2MA==",
    "MTE2Mzc5MzQ5",
    "MTE2MzQzNjY0",
    "NzA4MDgzNTc0",
};
void play(char* id);
char* down_playlist(char* id);
int read_file(char* playlist) {
    char* begin = strstr(playlist, "track_playlist");
    if (begin == (char*)0) {
        set_fg(240, 0, 0);
        put_string("[ERROR] Playlist find begin error.");
        restore_fg();
        exit(0);
    }
    // put_string("Begin: "); put_hex(begin); put_string("\n");
    char* end = strstr(begin, "song-list-pre-data");
    if (end == (char*)0) {
        set_fg(240, 0, 0);
        put_string("[ERROR] Playlist find end error.");
        restore_fg();
        exit(0);
    }
    // put_string("End: "); put_hex(end); put_string("\n");
    int c;
    char* ptr = begin;
    char* ids = malloc(32);
    *ids = '\0';
    for (;;) {
        put_string("Searching...\n");
        char* ptr2 = strstr(ptr, "href=\"/song?id=");
        *ids = '\0';
        if (ptr2) {
            // put_string("Ptr2: "); put_hex(ptr2); put_string("\n");
            char* ptr3 = strstr(ptr2 + 15, "\"");
            if (ptr3) {
                // put_string("Ptr3: "); put_hex(ptr3); put_string("\n");
                int len = ((int)(ptr3 - ptr2)) - 15;
                strncpy(ids, ptr2 + 15, len);
                ids[len] = '\0';
                ptr = ptr3;
            }
        }
        if (*ids < '0' || *ids > '9') {
            break;
        }
        else {
            put_string("Id: "); put_string(ids); put_string("\n");
            play(ids);
        }
    }
}
int main(int argc, char** argv) {
    char* list; int listL;
    int j, k = 0;
    for (j = 0; j < sizeof(list64) / sizeof(char*); j++) {
        char* str = format("%d) %s\n", j + 1, list64_name[j]);
        put_string(str);
        free(str);
    }
    if (argc <= 1) {
        put_string("请输入要播放的歌单编号：");
        sleep(1000);
        char buf[255];
        input(&buf, 255);
        k = atoi32(&buf);
        k--;
    }
    else {
        k = atoi32(argv[1]);
        k--;
    }
    if (k < 0 || k >= sizeof(list64) / sizeof(char*))
        k = 0;
    char* sh = format("echo %s | base64_decode > /tmp/163_list", list64[k]);
    shell(sh);
    free(sh);
    if (readfile("/tmp/163_list", &list, &listL) != 0) {
        return -1;
    }
    char* playlist = down_playlist(list);
    free(list);
    if (playlist == (char*)0) {
        set_fg(240, 0, 0);
        put_string("[ERROR] Download playlist failed.");
        restore_fg();
        exit(0);
    }
    read_file(playlist);
    return 0;
}

void play(char* id) {
    char* sh = format("gui_play 0 %s", id);
    shell(sh);
    free(sh);
    sleep(2000);
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