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

// Global grid optimization
vector<string> banana_grid;

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
        // Reserve space for SCREEN_HEIGHT lines, each SCREEN_WIDTH + newline
        buffer.resize(SCREEN_HEIGHT * (SCREEN_WIDTH + 1));
    }
    
    void clear() {
        // Reset buffer to spaces and newlines
        // We can optimize this by only resetting the 'content' parts if we wanted, 
        // but memset/fill is fast.
        fill(buffer.begin(), buffer.end(), ' ');
        for (int i = 0; i < SCREEN_HEIGHT; i++) {
            buffer[i * (SCREEN_WIDTH + 1) + SCREEN_WIDTH] = '\n';
        }
    }
    
    // Direct access
    char* raw_data() {
        return &buffer[0];
    }
    
    void set(int y, int x, char ch) {
        buffer[y * (SCREEN_WIDTH + 1) + x] = ch;
    }
    
    void print() {
        // Using printf for raw speed with C-string or cout with write
        // Since we are in C++, let's use cout.write
        cout << "\x1b[H\x1b[33m";
        cout.write(buffer.data(), buffer.size());
        cout << "\x1b[0m";
    }
};

void render_frame(double A, double B, Output& output) {
    output.clear();
    
    double cos_theta = cos(-A);
    double sin_theta = sin(-A);
    
    // Convert trigonometry results to fixed-point integers (16.16 format)
    int cos_fp = (int)(cos_theta * FIXED_ONE);
    int sin_fp = (int)(sin_theta * FIXED_ONE);
    
    // Precompute constants in fixed point
    int center_x_fp = (int)(center_x * FIXED_ONE);
    int center_y_fp = (int)(center_y * FIXED_ONE);
    
    // const_x = cx * (1 - cos) + cy * sin
    int const_x_fp = center_x_fp - ((long long)center_x_fp * cos_fp >> FIXED_SHIFT) + ((long long)center_y_fp * sin_fp >> FIXED_SHIFT);
    int const_y_fp = center_y_fp - ((long long)center_y_fp * cos_fp >> FIXED_SHIFT) - ((long long)center_x_fp * sin_fp >> FIXED_SHIFT);
    
    for (int yp = 0; yp < SCREEN_HEIGHT; yp++) {
        // Calculate row start using fixed point math
        int row_start_x_fp = -(yp * sin_fp) + const_x_fp;
        int row_start_y_fp =  (yp * cos_fp) + const_y_fp;
        
        // Initialize running coordinates
        int running_x_fp = row_start_x_fp;
        int running_y_fp = row_start_y_fp;
        
        for (int xp = 0; xp < SCREEN_WIDTH; xp++) {
            // Bit-shift right to get the integer part
            int src_x = running_x_fp >> FIXED_SHIFT;
            int src_y = running_y_fp >> FIXED_SHIFT;
            
            // If the coordinate hits the banana image, draw it
            if (src_x >= 0 && src_x < max_len && src_y >= 0 && src_y < num_lines) {
                if (src_x < banana_grid[src_y].length()) {
                     output.set(yp, xp, banana_grid[src_y][src_x]);
                }
            }
            
            // Advance the coordinate calculator by one pixel step
            running_x_fp += cos_fp;
            running_y_fp += sin_fp;
        }
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
    banana_grid = banana_lines;
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
