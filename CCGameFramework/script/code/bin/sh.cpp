#include "/include/exec"
#include "/include/io"
#include "/include/fs"
#include "/include/memory"
#include "/include/proc"
#include "/include/string"
#include "/include/shell"
#include "/include/sys"
#include "/include/gui"
struct node {
    char *text;
    node *prev;
    node *next;
};
void push(node **head, char *text) {
    if ((*head != (node*)0) && strcmp((*head)->text, text) == 0) {
        return;
    }
    node *new_node = (node *) malloc(sizeof(node));
    int len = strlen(text);
    char *new_text = malloc(len + 1);
    strcpy(new_text, text);
    new_node->text = new_text;
    new_node->prev = 0;
    new_node->next = *head;
    if (new_node->next)
        new_node->next->prev = new_node;
    *head = new_node;
}
int print_history(node *head) {
    set_fg(240, 200, 220);
    put_string("Command History:\n\n");
    while (head) {
        put_string(head->text);
        put_string("\n");
        head = head->next;
    }
}
int main(int argc, char **argv) {
    int i, j, state = 1, direct_input = input_state();
    char *text = malloc(256);
    char *_whoami = malloc(100);
    char *_hostname = malloc(100);
    char *_pwd = malloc(100);
    node* head = (node*)0;
    node *cur;
    switch_task();
    path_add("/usr");
    int show_text = 1;
    while (state) {
        gui_reset();
        if (direct_input && show_text) {
            set_fg(143, 164, 174);
            put_string("[");
            set_fg(63, 234, 53);
            whoami(_whoami);
            put_string(_whoami);
            put_string("@");
            hostname(_hostname);
            put_string(_hostname);
            put_string(" ");
            set_fg(73, 240, 229);
            pwd(_pwd);
            put_string(_pwd);
            set_fg(143, 164, 174);
            put_string("]# ");
            restore_fg();
            sleep(100);
        }
        if (!show_text)
            show_text = 1;
        *text = 0;
        while (state) {
            state = input(text, 100);
            if (state > INPUT_BEGIN)
                break;
            if (cur) {
                switch (state) {
                    case INPUT_UP:
                        if (cur->next)
                            cur = cur->next;
                        strcpy(text, cur->text);
                        break;
                    case INPUT_DOWN:
                        if (cur->prev)
                            cur = cur->prev;
                        strcpy(text, cur->text);
                        break;
                }
            } else {
                if (head) {
                    cur = head;
                    strcpy(text, cur->text);
                }
            }
        }
        if (strlen(text) == 0) {
            show_text = 0;
            continue;
        }
        if (strcmp(text, "exit") == 0)
            break;
        if (strcmp(text, "history") == 0) {
            print_history(head);
            continue;
        }
        if (strcmp(text, "?") == 0) {
            shell("help");
            continue;
        }
        push(&head, text);
        cur = 0;
        int pid = shell(text);
        if (pid < 0) {
            switch (pid) {
            case -1:
                set_fg(240, 0, 0);
                put_string("[ERROR] File not exists.\n");
                restore_fg();
                break;
            case -2:
                set_fg(240, 0, 0);
                put_string("[ERROR] Compile failed.\n");
                restore_fg();
                break;
            case -3:
                set_fg(240, 0, 0);
                put_string("[ERROR] Cannot execute system programs!\n");
                restore_fg();
                break;
            }
        }
        if (!direct_input)
            break;
        newline();
    }
    free(text);
    free(_whoami);
    free(_hostname);
    free(_pwd);
    return 0;
}