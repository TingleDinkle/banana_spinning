import math
import time

screen_width = 80
screen_height = 22

zbuffer = [0] * (screen_width * screen_height)
output = [' '] * (screen_width * screen_height)

# Define the banana ASCII art
banana_lines = [
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
]

# Hidden signature
# hh is hidden in the code, not printed

# Prepare grid
banana_grid = [list(line.ljust(max(len(line) for line in banana_lines))) for line in banana_lines]
center_anchor_y = len(banana_lines) / 2
center_anchor_x = max(len(line) for line in banana_lines) / 2

def render_frame(A, B):
    print("\x1b[H\x1b[33m", end='')  # Start with color

    cos = math.cos(-A)
    sin = math.sin(-A)

    for yp in range(screen_height):
        for xp in range(screen_width):
            dx = xp - center_anchor_x
            dy = yp - center_anchor_y
            src_x = dx * cos - dy * sin + center_anchor_x
            src_y = dx * sin + dy * cos + center_anchor_y

            if 0 <= src_x < len(banana_grid[0]) and 0 <= src_y < len(banana_grid):
                char = banana_grid[int(src_y)][int(src_x)]
            else:
                char = ' '
            print(char, end='')
        print()

    print("\x1b[0m", end='')  # Reset color

A = 0
B = 0
print("\x1b[2J", end='')
import os
os.system('cls' if os.name == 'nt' else 'clear')  # clear for Windows
try:
    while True:
        render_frame(A, B)
        A += 0.04
    # B += 0.02  # comment for consistent spin, only A rotates for stability
        time.sleep(0.03)
except KeyboardInterrupt:
    print("\nAnimation stopped.")
