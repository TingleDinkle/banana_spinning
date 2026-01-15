#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstring>
#include <csignal>

#if defined(_WIN32)
#include <windows.h>
#define SLEEP(x) Sleep(x)
#else
#include <unistd.h>
#define SLEEP(x) usleep(x * 1000)
#endif

using namespace std;

#define PI 3.1415926535
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 22
#define FIXED_SHIFT 16
#define FIXED_ONE (1 << FIXED_SHIFT)

vector<string> banana_lines = {
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

size_t num_lines = banana_lines.size();
size_t max_len = 0;
// Note: Loop initialization moved inside main or kept if it works in global scope (it does for this structure)
// We will refine global init safely.

// Optimize grid memory layout for cache locality.
string banana_grid_flat;
int banana_stride; 

double center_y, center_x;

void signalHandler(int signum) {
    cout << "\nAnimation stopped.\n";
    cout << "\x1b[0m";
    exit(signum);
}

class Output {
public:
    string buffer;
    Output() {
        // Reserve buffer space: Width + Newline * Height
        buffer.resize(SCREEN_HEIGHT * (SCREEN_WIDTH + 1));
    }
    
    void clear() {
        fill(buffer.begin(), buffer.end(), ' ');
        for (int i = 0; i < SCREEN_HEIGHT; i++) {
            buffer[i * (SCREEN_WIDTH + 1) + SCREEN_WIDTH] = '\n';
        }
    }
    
    // Direct pointer access for speed
    char* raw_data() {
        return &buffer[0];
    }
    
    void set(int y, int x, char ch) {
        buffer[y * (SCREEN_WIDTH + 1) + x] = ch;
    }
    
    void print() {
        // Fast output using cout.write
        cout << "\x1b[H\x1b[33m";
        cout.write(buffer.data(), buffer.size());
        cout << "\x1b[0m";
    }
};

void render_frame(double A, double B, Output& output) {
    output.clear();
    
    double cos_theta = cos(-A);
    double sin_theta = sin(-A);
    
    // Use fixed-point math (16.16) for performance.
    int cos_fp = (int)(cos_theta * FIXED_ONE);
    int sin_fp = (int)(sin_theta * FIXED_ONE);
    
    // Convert center coordinates to fixed-point.
    int center_x_fp = (int)(center_x * FIXED_ONE);
    int center_y_fp = (int)(center_y * FIXED_ONE);
    
    // Optimize: Bounding Box rendering to skip empty space.
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
    
    // Clamp bounding box to screen dimensions.
    if (min_xp < 0) min_xp = 0;
    if (max_xp >= SCREEN_WIDTH) max_xp = SCREEN_WIDTH - 1;
    if (min_yp < 0) min_yp = 0;
    if (max_yp >= SCREEN_HEIGHT) max_yp = SCREEN_HEIGHT - 1;

    // Precalculate row start constants.
    int const_x_fp = center_x_fp - ((long long)center_x_fp * cos_fp >> FIXED_SHIFT) + ((long long)center_y_fp * sin_fp >> FIXED_SHIFT);
    int const_y_fp = center_y_fp - ((long long)center_y_fp * cos_fp >> FIXED_SHIFT) - ((long long)center_x_fp * sin_fp >> FIXED_SHIFT);
    
    int banana_w = (int)max_len;
    int banana_h = (int)num_lines;

    // Render within the bounding box.
    for (int yp = min_yp; yp <= max_yp; yp++) {
        int row_start_x_fp = -(yp * sin_fp) + const_x_fp;
        int row_start_y_fp =  (yp * cos_fp) + const_y_fp;
        
        int running_x_fp = row_start_x_fp + min_xp * cos_fp;
        int running_y_fp = row_start_y_fp + min_xp * sin_fp;
        
        int xp = min_xp;

        // Direct pointer access for speed.
        char* row_ptr = &output.buffer[yp * (SCREEN_WIDTH + 1) + min_xp];

        // Manual loop unrolling (4x) to reduce loop overhead.
        #define RENDER_PIXEL_CPP \
            { \
                int src_x = running_x_fp >> FIXED_SHIFT; \
                int src_y = running_y_fp >> FIXED_SHIFT; \
                if ((unsigned int)src_x < (unsigned int)banana_w && \
                    (unsigned int)src_y < (unsigned int)banana_h) { \
                    *row_ptr = banana_grid_flat[src_y * banana_stride + src_x]; \
                } \
                row_ptr++; \
                running_x_fp += cos_fp; \
                running_y_fp += sin_fp; \
            }
        
        for (; xp <= max_xp - 3; xp += 4) {
            RENDER_PIXEL_CPP
            RENDER_PIXEL_CPP
            RENDER_PIXEL_CPP
            RENDER_PIXEL_CPP
        }

        // Handle remaining pixels.
        for (; xp <= max_xp; xp++) {
            RENDER_PIXEL_CPP
        }
        
        #undef RENDER_PIXEL_CPP
    }
    
    output.print();
}

int main() {
    signal(SIGINT, signalHandler);

    // Fast IO
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    // Init Logic
    for (const auto& line : banana_lines) { max_len = max(max_len, line.length()); }
    
    // Flatten the grid
    banana_stride = max_len;
    banana_grid_flat.resize(num_lines * banana_stride, ' '); // Fill with spaces initially
    
    for(size_t i=0; i<num_lines; i++) {
        // Copy each line into the flat buffer
        for(size_t j=0; j<banana_lines[i].length(); j++) {
            banana_grid_flat[i * banana_stride + j] = banana_lines[i][j];
        }
    }

    center_y = num_lines / 2.0;
    center_x = max_len / 2.0;

    cout << "\x1b[2J";
    cout << "\x1b[H";
    
    Output output;
    
    double A = 0.0;
    double B = 0.0;
    
    while (true) {
        render_frame(A, B, output);
        A += 0.04;
        SLEEP(30);
    }
    
    return 0;
}
