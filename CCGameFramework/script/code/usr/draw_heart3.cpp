#include "/include/io"
#include "/include/math"
double f(double x, double y, double z) {
    double a = x * x + 9.0 / 4.0 * y * y + z * z - 1;
    return a * a * a - x * x * z * z * z - 9.0 / 80.0 * y * y * z * z * z;
}
double h(double x, double z) {
    double y;
    for (y = 1.0; y >= 0.0; y -= 0.001)
        if (f(x, y, z) <= 0.0)
            return y;
    return 0.0;
}
int main(int argc, char **argv) {
    double x, z;
    for (z = 1.5; z > -1.5; z -= 0.1) {
        for (x = -1.5; x < 1.5; x += 0.05) {
            double v = f(x, 0.0, z);
            if (v <= 0.0) {
                double y0 = h(x, z);
                double ny = 0.01;
                double nx = h(x + ny, z) - y0;
                double nz = h(x, z + ny) - y0;
                double nd = 1.0 / sqrt(nx * nx + ny * ny + nz * nz);
                double d = (nx + ny - nz) * nd * 0.5 + 0.5;
                put_char(".:-=+*#%@"[(int)(d * 5.0)]);
            } else {
                put_char(' ');
            }
        }
        put_char('\n');
    }
    return 0;
}