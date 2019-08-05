#include "/include/io"
#include "/include/memory"
#include "/include/shell"
#include "/include/readfile"
#include "/include/json"
int main(int argc, char **argv) {
    shell("/usr/http_get v1.hitokoto.cn | /usr/api_json");
    char* json; int len;
    if (readfile("/http/v1.hitokoto.cn", &json, &len) == 0) {
        json_object* obj = json_parse_obj(json);
        put_string("\n");
        put_string(json_obj_get_string(obj, "hitokoto")->data.str);
    }
    return 0;
}