#include "/include/io"
#include "/include/math"
int main(int argc, char **argv) {
    double x, y;
    for (y = 1.5; y > -1.5; y -= 0.1) {
        for (x = -1.5; x < 1.5; x += 0.05) {
            double z = x * x + y * y - 1.0;
            double f = z * z * z - x * x * y * y * y;
            put_char(f < 0.0 ? ".:-=+*#%@"[(int)(f * -8.0)] : ' ');
        }
        put_char('\n');
    }
    return 0;
}