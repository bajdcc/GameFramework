//
// Project: clibparser
// Created by bajdcc
//

// 系统属性
int hostname(char *s) {
    s;
    interrupt 5;
}
int set_cycle(int cycle) {
    cycle;
    interrupt 59;
}
int reset_cycle(int cycle) {
    set_cycle(0);
}
long timestamp() {
    interrupt 102;
}
