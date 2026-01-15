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
    
    // Convert trigonometry results to fixed-point integers (16.16 format)
    int cos_fp = (int)(cos_theta * FIXED_ONE);
    int sin_fp = (int)(sin_theta * FIXED_ONE);
    
    // Precompute constants in fixed point
    int center_x_fp = (int)(center_x * FIXED_ONE);
    int center_y_fp = (int)(center_y * FIXED_ONE);
    
    // --- BOUNDING BOX OPTIMIZATION ---
    // We only need to render pixels that might actually contain the banana.
    // Project the 4 corners of the grid (0,0), (W,0), (0,H), (W,H) to screen space.
    // Forward Transform: 
    // xp = (gx - cx) * cos(A) - (gy - cy) * sin(A) + cx
    // yp = (gx - cx) * sin(A) + (gy - cy) * cos(A) + cy
    // Note: Our render loop uses -A (inverse), so Forward uses A.
    // cos(A) = cos(-(-A)) = cos_theta
    // sin(A) = sin(-(-A)) = -sin_theta
    
    // Corners relative to center (gx-cx, gy-cy)
    double cx = center_x;
    double cy = center_y;
    double w = (double)max_len;
    double h = (double)num_lines;
    
    // 4 Corners: Top-Left, Top-Right, Bot-Left, Bot-Right
    double corners_x[] = {-cx, w - cx, -cx, w - cx};
    double corners_y[] = {-cy, -cy, h - cy, h - cy};
    
    int min_xp = SCREEN_WIDTH, max_xp = 0;
    int min_yp = SCREEN_HEIGHT, max_yp = 0;
    
    // Use the same trig values (cos_theta, sin_theta) but aware of the sign change for forward transform
    // Forward Rotation Matrix:
    // | cos  -sin |
    // | sin   cos |
    // But our variables store cos(-A)=cos(A) and sin(-A)=-sin(A).
    // So cos(A) = cos_theta, sin(A) = -sin_theta.
    
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
    
    // Clamp to screen bounds
    if (min_xp < 0) min_xp = 0;
    if (max_xp >= SCREEN_WIDTH) max_xp = SCREEN_WIDTH - 1;
    if (min_yp < 0) min_yp = 0;
    if (max_yp >= SCREEN_HEIGHT) max_yp = SCREEN_HEIGHT - 1;

    // --- RENDER LOOP (Fixed Point + Bounding Box) ---

    // const_x = cx * (1 - cos) + cy * sin
    int const_x_fp = center_x_fp - ((long long)center_x_fp * cos_fp >> FIXED_SHIFT) + ((long long)center_y_fp * sin_fp >> FIXED_SHIFT);
    int const_y_fp = center_y_fp - ((long long)center_y_fp * cos_fp >> FIXED_SHIFT) - ((long long)center_x_fp * sin_fp >> FIXED_SHIFT);

    // Iterate only within the bounding box
    for (int yp = min_yp; yp <= max_yp; yp++) {
        // Calculate row start using fixed point math
        int row_start_x_fp = -(yp * sin_fp) + const_x_fp;
        int row_start_y_fp =  (yp * cos_fp) + const_y_fp;
        
        // Offset running coordinates to the start of the bounding box (min_xp)
        // We can't just start at 0, we must advance the "running" vars to min_xp
        int running_x_fp = row_start_x_fp + min_xp * cos_fp;
        int running_y_fp = row_start_y_fp + min_xp * sin_fp;
        
        int row_offset = yp * (SCREEN_WIDTH + 1);

        for (int xp = min_xp; xp <= max_xp; xp++) {
            // Bit-shift right to get the integer part
            int src_x = running_x_fp >> FIXED_SHIFT;
            int src_y = running_y_fp >> FIXED_SHIFT;

            // Check if the rotated pixel lands on the banana
            if (src_x >= 0 && src_x < max_len && src_y >= 0 && src_y < num_lines) {
                output[row_offset + xp] = banana_grid[src_y * max_len + src_x];
            }
            
            // Move to the next pixel
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