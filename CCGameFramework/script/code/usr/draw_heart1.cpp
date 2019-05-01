#include "/include/io"
#include "/include/math"
int main(int argc, char **argv) {
    double x, y;
    for (y = 1.5; y > -1.5; y -= 0.1) {
        for (x = -1.5; x < 1.5; x += 0.05) {
            double a = x * x + y * y - 1.0;
            put_char(a * a * a - x * x * y * y * y <= 0.0 ? '*' : ' ');
        }
        put_char('\n');
    }
    return 0;
}