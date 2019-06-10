#include "/include/shell"
int main(int argc, char** argv) {
    shell("cat /usr/test_command.txt | bat");
    return 0;
}