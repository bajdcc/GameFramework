#include "/include/io"
#include "/include/fs"
int read_file(int handle) {
    int c;
    while (c = read(handle), c < 0x1000) {
        put_char(c);
    }
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
}
int main(int argc, char **argv) {
    if (argc > 1) {
        int handle = open(argv[1]);
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
    }
    return 0;
}