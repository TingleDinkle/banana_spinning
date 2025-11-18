#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#define SLEEP(x) Sleep(x)
#else
#include <unistd.h>
#define SLEEP(x) usleep(x * 1000)
#endif

#define PI 3.1415926535
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 22

char output[SCREEN_WIDTH * SCREEN_HEIGHT];

const char *banana_lines[] = {
    "_",
    "//\\",
    "V  \\",
    " \\  \\_",
    "  \\,'.`-.",
    "   |\\ `. `.       ",
    "   ( \\  `. `-.                        _,.-:\\",
    "    \\ \\   `.  `-._             __..--' ,-';/",
    "     \\ `.   `-.   `-..___..---'   _.--' ,'/",
    "      `. `.    `-._        __..--'    ,' /",
    "        `. `-_     ``--..''       _.-' ,'",
    "          `-_ `-.___        __,--'   ,'",
    "             `-.__  `----\"\"\"    __.-'",
    "                `--..____..--'",
};

size_t num_lines = sizeof(banana_lines) / sizeof(banana_lines[0]);

// Allocate grid
char **banana_grid = NULL;

void init_grid() {
    banana_grid = malloc(num_lines * sizeof(char *));
    if (!banana_grid) exit(1);
    for (size_t i = 0; i < num_lines; i++) {
        size_t len = strlen(banana_lines[i]);
        banana_grid[i] = malloc((len + 1) * sizeof(char));
        if (!banana_grid[i]) exit(1);
        strcpy(banana_grid[i], banana_lines[i]);
        // Pad to max length? skip for simplicity
    }
}

void free_grid() {
    if (banana_grid) {
        for (size_t i = 0; i < num_lines; i++) {
            if (banana_grid[i]) free(banana_grid[i]);
        }
        free(banana_grid);
        banana_grid = NULL;
    }
}

size_t max_len = 0;
void get_max_len() {
    for (size_t i = 0; i < num_lines; i++) {
        size_t len = strlen(banana_lines[i]);
        if (len > max_len) max_len = len;
    }
}

double center_y, center_x;

void render_frame(double A, double B) {

    // Clear output
    memset(output, ' ', SCREEN_WIDTH * SCREEN_HEIGHT);

    double cos_theta = cos(-A);
    double sin_theta = sin(-A);

    for (int yp = 0; yp < SCREEN_HEIGHT; yp++) {
        for (int xp = 0; xp < SCREEN_WIDTH; xp++) {
            double dx = xp - center_x;
            double dy = yp - center_y;
            double src_x = dx * cos_theta - dy * sin_theta + center_x;
            double src_y = dx * sin_theta + dy * cos_theta + center_y;

            if (src_x >= 0 && src_x < max_len && src_y >= 0 && src_y < num_lines) {
                char ch = banana_grid[(int)src_y][(int)src_x];
                output[yp * SCREEN_WIDTH + xp] = ch;
            } else {
                output[yp * SCREEN_WIDTH + xp] = ' ';
            }
        }
    }

    // Add yellow color
    printf("\x1b[H\x1b[33m");
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        putchar(output[i]);
        if ((i + 1) % SCREEN_WIDTH == 0) putchar('\n');
    }
    printf("\x1b[0m");
}

int main() {
    get_max_len();
    init_grid();
    center_y = num_lines / 2.0;
    center_x = max_len / 2.0;

    printf("\x1b[2J");
    printf("\x1b[H");

    double A = 0.0;
    double B = 0.0; // not used

    for (;; ) {
        render_frame(A, B);
        A += 0.04;
        SLEEP(30); // 30ms
    }

    free_grid();
    return 0;
}
