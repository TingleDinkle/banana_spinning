#include <math.h>
#include <stdio.h>
#include <string.h>

#define PI 3.1415926535
#define screen_width 80
#define screen_height 22

float zbuffer[screen_width * screen_height];
char output[screen_width * screen_height];

void render_frame(float A, float B) {
    memset(output, ' ', screen_width * screen_height);
    memset(zbuffer, 0, screen_width * screen_height * sizeof(float));

    float cosA = cos(A), sinA = sin(A);
    float cosB = cos(B), sinB = sin(B);

    for (float t = 0; t < 2 * PI; t += 0.03) {
        float x_init = (2 + 1 * cos(2 * t)) * cos(3 * t);
        float y_init = (2 + 1 * cos(2 * t)) * sin(3 * t);
        float z_init = 1 * sin(2 * t);

        float x = x_init * cosB - y_init * sinB;
        float y = sinA * z_init + cosA * (x_init * sinB + y_init * cosB);
        float z = cosA * z_init - sinA * (x_init * sinB + y_init * cosB) + 5;
        float ooz = 1 / z;

        int xp = (int)(40 + 50 * ooz * x);
        int yp = (int)(11 + 15 * ooz * y);

        if (xp >= 0 && xp < screen_width && yp >= 0 && yp < screen_height) {
            int o = xp + yp * screen_width;
            if (ooz > zbuffer[o]) {
                zbuffer[o] = ooz;
                // Approximate illumination: directional light for visibility
                float L = (-x * 0.01 + y * 0.01) + 0.5;
                if (L > 0) {
                    int luminance_index = (int)(L * 12);
                    output[o] = ".,-~:;=!*#$@"[luminance_index > 11 ? 11 : luminance_index];
                }
            }
        }
    }

    // Now dump output[] to the screen.
    printf("\x1b[H");
    for (int i = 0; i < screen_width * screen_height; i++) {
        putchar(output[i]);
        if ((i + 1) % screen_width == 0) putchar('\n');
    }
}

int main() {
    float A = 0, B = 0;
    printf("\x1b[2J");
    for (;;) {
        render_frame(A, B);
        A += 0.04;
        B += 0.02;
    }
    return 0;
}
