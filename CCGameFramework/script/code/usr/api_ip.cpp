#include "/include/io"
#include "/include/memory"
#include "/include/readfile"
#include "/include/json"
void help() {
    set_fg(240, 200, 220);
    put_string("\n");
    put_string("all            -  所有信息\n");
    put_string("country        - （中文）国家，如：中国\n");
    put_string("countryCode    - （英文）国家，如：CN\n");
    put_string("region         - （英文）省份，如：JS\n");
    put_string("regionName     - （中文）省份，如：江苏省\n");
    put_string("city           - （中文）城市，如：上海\n");
    put_string("zip            - （整数）邮政编码，如：100000\n");
    put_string("lat            - （浮点）经度\n");
    put_string("lon            - （浮点）纬度\n");
    put_string("timezone       - （英文）时区，如：Asia/Shanghai\n");
    put_string("isp            - （英文）互联网服务提供商\n");
    put_string("org            - （英文）组织\n");
    put_string("as             - （英文）自治系统\n");
    put_string("query          - （地址）IP地址\n");
    restore_fg();
}
int main(int argc, char** argv) {
    if (argc < 2) {
        set_fg(240, 0, 0);
        put_string("[ERROR] Miss argument.");
        restore_fg();
        help();
        return;
    }
    char* json; int len;
    if (readfile("/http/ip-api.com/json/?lang=zh-CN", &json, &len) == 0) {
        json_object* obj = json_parse_obj(json);
        json_object* status = json_obj_get_string(obj, "status");
        if (status == (json_object*)0 || strcmp(status->data.str, "success") != 0) {
            set_fg(240, 0, 0);
            put_string("[ERROR] Invalid response from server.");
            restore_fg();
        }
        else {
            if (strcmp(argv[1], "all") == 0) {
                int len = json_obj_size(obj);
                int i;
                for (i = 0; i < len; i++) {
                    json_object* k = json_obj_get_1(obj, i);
                    json_object* v = json_obj_get_2(obj, i);
                    put_string(k->data.str);
                    put_string(": ");
                    if (v->type == j_string)
                        put_string(v->data.str);
                    else if (v->type == j_double)
                        put_double(v->data.d);
                    put_string("\n");
                }
            }
            else {
                json_object* obj = json_obj_get_string(obj, argv[1]);
                if (obj) {
                    if (obj->type == j_string)
                        put_string(obj->data.str);
                    else if (obj->type == j_double)
                        put_double(obj->data.d);
                    else {
                        set_fg(240, 0, 0);
                        put_string("[ERROR] Unknown data type.");
                        restore_fg();
                    }
                }
                else {
                    set_fg(240, 0, 0);
                    put_string("[ERROR] Invalid argument.");
                    restore_fg();
                    help();
                }
            }
        }
    }
    return 0;
}