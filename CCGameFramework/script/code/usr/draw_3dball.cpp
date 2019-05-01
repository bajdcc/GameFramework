#include "/include/io"
#include "/include/math"
int main(int argc, char **argv) {
    double x, y;
    for (y = 1; y >= -1; y -= 0.07, put_char('\n'))
        for (x = -1; x <= 1; x += 0.035)
            put_char(x * x + y * y >= 1 ? 'M' : "@@%#*+=;:. "[(int) (
                    ((x + y + sqrt(1 - (x * x + y * y))) * 0.5773502692 + 1)
                    * 5.0 + 0.5)]);
    return 0;
}