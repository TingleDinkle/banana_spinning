#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <signal.h>

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
#define FIXED_SHIFT 16
#define FIXED_ONE (1 << FIXED_SHIFT)

// Output buffer includes newlines: (Width + 1 for '\n') * Height
char output[SCREEN_HEIGHT * (SCREEN_WIDTH + 1)];

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
size_t max_len = 0;

// Allocate flattened grid
char *banana_grid = NULL;

void cleanup_and_exit(int signo) {
    if (signo == SIGINT) {
        printf("\nAnimation stopped.\n");
    }
    if (banana_grid) free(banana_grid);
    printf("\x1b[0m"); // Reset color
    exit(0);
}

void init_grid() {
    // Find max length
    for (size_t i = 0; i < num_lines; i++) {
        size_t len = strlen(banana_lines[i]);
        if (len > max_len) max_len = len;
    }

    // Allocate contiguous block
    banana_grid = malloc(num_lines * max_len * sizeof(char));
    if (!banana_grid) exit(1);
    
    // Fill with spaces
    memset(banana_grid, ' ', num_lines * max_len);

    // Copy data
    for (size_t i = 0; i < num_lines; i++) {
        size_t len = strlen(banana_lines[i]);
        memcpy(&banana_grid[i * max_len], banana_lines[i], len);
    }
}

void free_grid() {
    if (banana_grid) {
        free(banana_grid);
        banana_grid = NULL;
    }
}

void get_max_len() {
   // Already handled in init_grid
}

double center_y, center_x;

void render_frame(double A, double B) {
    // Fill with spaces
    memset(output, ' ', sizeof(output));
    
    // Add newlines to buffer (could be done once if optimizing further, but memset clears it)
    for(int i=0; i<SCREEN_HEIGHT; i++) {
        output[i * (SCREEN_WIDTH + 1) + SCREEN_WIDTH] = '\n';
    }

    double cos_theta = cos(-A);
    double sin_theta = sin(-A);
    
    // We use "Fixed Point" math here. Instead of using slow decimals, we multiply 
    // everything by 65536 and use whole numbers. This is much faster for a computer.
    int cos_fp = (int)(cos_theta * FIXED_ONE);
    int sin_fp = (int)(sin_theta * FIXED_ONE);
    
    // We find the center of our banana so we can spin it around its middle.
    int center_x_fp = (int)(center_x * FIXED_ONE);
    int center_y_fp = (int)(center_y * FIXED_ONE);
    
    // To save time, we calculate exactly where the banana is on the screen.
    // We don't want to waste energy checking empty black space.
    double cx = center_x;
    double cy = center_y;
    double w = (double)max_len;
    double h = (double)num_lines;
    
    double corners_x[] = {-cx, w - cx, -cx, w - cx};
    double corners_y[] = {-cy, -cy, h - cy, h - cy};
    
    int min_xp = SCREEN_WIDTH, max_xp = 0;
    int min_yp = SCREEN_HEIGHT, max_yp = 0;
    
    double fwd_cos = cos_theta;
    double fwd_sin = -sin_theta;
    
    for(int i=0; i<4; i++) {
        double px = corners_x[i] * fwd_cos - corners_y[i] * fwd_sin + cx;
        double py = corners_x[i] * fwd_sin + corners_y[i] * fwd_cos + cy;
        
        if (px < min_xp) min_xp = (int)floor(px);
        if (px > max_xp) max_xp = (int)ceil(px);
        if (py < min_yp) min_yp = (int)floor(py);
        if (py > max_yp) max_yp = (int)ceil(py);
    }
    
    // We make sure our "box" doesn't go off the edge of the terminal.
    if (min_xp < 0) min_xp = 0;
    if (max_xp >= SCREEN_WIDTH) max_xp = SCREEN_WIDTH - 1;
    if (min_yp < 0) min_yp = 0;
    if (max_yp >= SCREEN_HEIGHT) max_yp = SCREEN_HEIGHT - 1;

    // These values help us map screen pixels back to our banana image.
    int const_x_fp = center_x_fp - ((long long)center_x_fp * cos_fp >> FIXED_SHIFT) + ((long long)center_y_fp * sin_fp >> FIXED_SHIFT);
    int const_y_fp = center_y_fp - ((long long)center_y_fp * cos_fp >> FIXED_SHIFT) - ((long long)center_x_fp * sin_fp >> FIXED_SHIFT);

    int banana_w = (int)max_len;
    int banana_h = (int)num_lines;

    // We start drawing row by row.
    for (int yp = min_yp; yp <= max_yp; yp++) {
        int row_start_x_fp = -(yp * sin_fp) + const_x_fp;
        int row_start_y_fp =  (yp * cos_fp) + const_y_fp;
        
        int running_x_fp = row_start_x_fp + min_xp * cos_fp;
        int running_y_fp = row_start_y_fp + min_xp * sin_fp;
        
        // We point directly to the spot in memory where this row starts.
        char *row_ptr = &output[yp * (SCREEN_WIDTH + 1) + min_xp];

        int xp = min_xp;
        // Optimization: We process 4 pixels at a time to keep the CPU busy.
        for (; xp <= max_xp - 3; xp += 4) {
            for (int i = 0; i < 4; i++) {
                int src_x = running_x_fp >> FIXED_SHIFT;
                int src_y = running_y_fp >> FIXED_SHIFT;

                // Check if we hit the banana. Using unsigned numbers lets us 
                // check "too small" and "too big" in a single step.
                if ((unsigned int)src_x < (unsigned int)banana_w && 
                    (unsigned int)src_y < (unsigned int)banana_h) {
                    *row_ptr = banana_grid[src_y * banana_w + src_x];
                }
                row_ptr++;
                running_x_fp += cos_fp;
                running_y_fp += sin_fp;
            }
        }
        // If there are a few pixels left over, we finish them here.
        for (; xp <= max_xp; xp++) {
            int src_x = running_x_fp >> FIXED_SHIFT;
            int src_y = running_y_fp >> FIXED_SHIFT;

            if ((unsigned int)src_x < (unsigned int)banana_w && 
                (unsigned int)src_y < (unsigned int)banana_h) {
                *row_ptr = banana_grid[src_y * banana_w + src_x];
            }
            row_ptr++;
            running_x_fp += cos_fp;
            running_y_fp += sin_fp;
        }
    }

    // Dump the whole frame to the terminal in one fast operation
    printf("\x1b[H\x1b[33m");
    fwrite(output, sizeof(char), sizeof(output), stdout);
    printf("\x1b[0m");
}

int main() {
    signal(SIGINT, cleanup_and_exit);
    
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