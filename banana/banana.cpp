#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstring>
#include <chrono>
#include <thread>

using namespace std;

#define PI 3.1415926535
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 22

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
for (auto& line : banana_lines) { max_len = max(max_len, line.length()); }

vector<string> banana_grid = banana_lines;

double center_y = num_lines / 2.0;
double center_x = max_len / 2.0;

class Output {
public:
    vector<string> buffer;
    Output() {
        buffer.resize(SCREEN_HEIGHT);
        for (int i = 0; i < SCREEN_HEIGHT; i++) {
            buffer[i] = string(SCREEN_WIDTH, ' ');
        }
    }
    
    void clear() {
        for (int i = 0; i < SCREEN_HEIGHT; i++) {
            fill(buffer[i].begin(), buffer[i].end(), ' ');
        }
    }
    
    char& at(int y, int x) {
        return buffer[y][x];
    }
    
    void print() {
        cout << "\x1b[H\x1b[33m";
        for (int i = 0; i < SCREEN_HEIGHT; i++) {
            cout << buffer[i] << '\n';
        }
        cout << "\x1b[0m";
    }
};

void render_frame(double A, double B, Output& output) {
    output.clear();
    
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
                output.at(yp, xp) = ch;
            }
        }
    }
    
    output.print();
}

int main() {
    cout << "\x1b[2J";
    cout << "\x1b[H";
    
    Output output;
    
    double A = 0.0;
    double B = 0.0;
    
    while (true) {
        render_frame(A, B, output);
        A += 0.04;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    
    return 0;
}
