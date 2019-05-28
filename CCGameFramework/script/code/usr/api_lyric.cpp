#include "/include/proc"
#include "/include/fs"
#include "/include/io"
#include "/include/memory"
#include "/include/string"
#include "/include/json"
struct string {
    char* text;
    int capacity;
    int length;
};
string new_string() {
    string s;
    s.text = malloc(16);
    s.capacity = 16;
    s.length = 0;
    return s;
}
void append_char(string* s, char c) {
    if (s->length >= s->capacity - 1) {
        s->capacity <<= 1;
        char* new_text = malloc(s->capacity);
        strcpy(new_text, s->text);
        free(s->text);
        s->text = new_text;
    }
    (s->text)[s->length++] = c;
    (s->text)[s->length] = 0;
}
void read_file(int handle) {
    int cmd = 0;
    string s = new_string();
    int c;
    while (c = read(handle), c < 0x1000) {
        if (((char)c) == '\033') {
            append_char(&s, c);
            cmd = 1 - cmd;
        }
        else if (cmd == 0) {
            append_char(&s, c);
        }
    }
    put_string(json_parse(s.text));
}
int main(int argc, char **argv) {
    if (argc <= 1) {
        return 0;
    }
    char* cmd = malloc(256);
    strcpy(cmd, "/http/music.163.com/api/song/media?id=");
    strcat(cmd, argv[1]);
    int handle = open(cmd);
    switch (handle) {
    default:
        // put_string("[INFO] Success.");
        read_file(handle);
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
    return 0;
}