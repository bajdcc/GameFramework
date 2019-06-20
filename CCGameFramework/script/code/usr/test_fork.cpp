#include "/include/io"
#include "/include/proc"
#include "/include/memory"
#include "/include/string"
int main(int argc, char** argv) {
    int i;
    put_string("========== [#2 TEST FORK] ==========\n");
    put_string("Command:");
    for (i = 0; i < argc; ++i) {
        put_string(" ");
        put_string(argv[i]);
    }
    put_string("\n");
    char* str = malloc(100); strcpy(str, "Child: fork return ");
    char* str2 = malloc(100); strcpy(str2, "Parent: fork return ");
    i = fork();
    if (i == -1) {
        sleep(500);
        put_string(str); put_int(i); put_string("\n");
    }
    else {
        wait();
        put_string(str2); put_int(i); put_string("\n");
        put_string("========== [#2 TEST FORK] ==========\n");
    }
    return 0;
}