#include "/include/exec"
#include "/include/io"
#include "/include/fs"
#include "/include/memory"
#include "/include/proc"
#include "/include/string"
#include "/include/shell"
#include "/include/sys"
int process(char *text) {
    char *tmp = malloc(256), c;
    int i = 0, j = 0;
    while (true) {
        c = text[i];
        if (c == '>') {
            i++;
            if (text[i] == '>') { // append
                i++;
                strcpy(tmp + j, "| append ");
                j += 9;
            } else { // truncate
                strcpy(tmp + j, "| write ");
                j += 8;
            }
        }
        tmp[j++] = text[i++];
        if (c == (char) 0)
            break;
    }
    strcpy(text, tmp);
}
int exec_single(char *text, int *total) {
    while (*text == ' ')
        text++;
    if (strncmp(text, "/sys/", 5) == 0) {
        return -3;
    }
    (*total)++;
    return exec_sleep(text);
}
int exec_start(char *text, int *total) {
    char *c = strchr(text, '|');
    if (c == (char *) 0) {
        return exec_single(text, total);
    } else {
        *c++ = '\0';
        if (*c == '\0')
            return exec_single(text, total);
        int right = exec_start(c, total);
        if (right < 0)
            return right;
        int left = exec_single(text, total);
        exec_connect(left, right);
        exec_wakeup(right);
        return left;
    }
}
struct node {
    char *text;
    node *prev;
    node *next;
};
node *push(node **head, char *text) {
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
    return new_node;
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
    int i, j, total, state = 1, direct_input = input_state();
    char *text = malloc(256);
    char *_whoami = malloc(100);
    char *_hostname = malloc(100);
    char *_pwd = malloc(100);
    node *head;
    node *cur;
    while (state) {
        total = 0;
        if (direct_input) {
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
        }
        *text = 0;
        while (true) {
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
        if (strlen(text) == 0)
            continue;
        if (strcmp(text, "exit") == 0)
            break;
        if (strcmp(text, "history") == 0) {
            print_history(head);
            continue;
        }
        if (strcmp(text, "?") == 0) {
            switch_task();
            shell("help");
            switch_task();
            continue;
        }
        push(&head, text);
        cur = 0;
        process(text);
        switch_task();
        int pid = exec_start(text, &total);
        if (pid >= 0) {
            exec_wakeup(pid);
            for (j = 0; j < total; ++j) {
                wait();
            }
        } else {
            exec_kill_children();
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
        switch_task();
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