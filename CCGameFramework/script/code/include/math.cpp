//
// Project: clibparser
// Created by bajdcc
//

// 数学函数（为了精度，默认先做double）
double sqrt(double number) {
    number;
    interrupt 201;
}

// 返回[0,count)间的随机数
int rand(int count) {
    count;
    interrupt 202;
}

struct __math_2d_struct__ {
    double x, y;
};

double atan2(double x, double y) {
    __math_2d_struct__ s;
    s.x = x;
    s.y = y;
    &s;
    interrupt 203;
}

double fabs(double number) {
    number;
    interrupt 204;
}

double fmod(double x, double y) {
    __math_2d_struct__ s;
    s.x = x;
    s.y = y;
    &s;
    interrupt 205;
}

double pow(double x, double y) {
    __math_2d_struct__ s;
    s.x = x;
    s.y = y;
    &s;
    interrupt 206;
}

double sin(double number) {
    number;
    interrupt 207;
}

double cos(double number) {
    number;
    interrupt 208;
}