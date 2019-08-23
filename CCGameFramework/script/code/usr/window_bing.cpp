#include "/include/shell"
#include "/include/proc"
#include "/include/readfile"
#include "/include/json"
int main(int argc, char** argv) {
    if (argc <= 1) {
        set_fg(240, 0, 0);
        put_string("[Error] Missing argument.");
        restore_fg();
        return 1;
    }
    char* buf = malloc(512);
    strcpy(buf, "/http/www.bing.com/HPImageArchive.aspx?format=js&n=1&idx=");
    strcat(buf, argv[1]);
    put_string("Reading API: ");
    put_string(buf);
    put_string("\n");
    char* out; int n;
    if (readfile(buf, &out, &n) != 0) {
        set_fg(240, 0, 0);
        put_string("[Error] Bing API failed.");
        restore_fg();
        return 2;
    }
    json_object* obj = json_parse_obj(out);
    strcpy(buf, "window_image /http/bin!www.bing.com");
    strcat(buf, json_obj_get_string(json_array_get(json_obj_get_string(obj, "images"), 0), "url")->data.str);
    put_string("#");
    put_string(buf);
    put_string("\n");
    shell(buf);
    return 0;
}
