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
    cos = math.cos(-A)
    sin = math.sin(-A)

    # Precompute constants to save ops inside the loop
    # src_x = (xp - cx) * cos - (yp - cy) * sin + cx
    #       = xp*cos - yp*sin + (cx - cx*cos + cy*sin)
    const_x = center_anchor_x * (1 - cos) + center_anchor_y * sin
    const_y = center_anchor_y * (1 - cos) - center_anchor_x * sin
    
    frame_chars = []
    # Add color and home cursor
    frame_chars.append("\x1b[H\x1b[33m")
    
    banana_height = len(banana_grid)
    banana_width = len(banana_grid[0])

    for yp in range(screen_height):
        # Precompute Y component for this row
        row_term_x = -yp * sin + const_x
        row_term_y = yp * cos + const_y
        
        for xp in range(screen_width):
            src_x = int(xp * cos + row_term_x)
            src_y = int(xp * sin + row_term_y)

            if 0 <= src_x < banana_width and 0 <= src_y < banana_height:
                frame_chars.append(banana_grid[src_y][src_x])
            else:
                frame_chars.append(' ')
        frame_chars.append('\n')

    frame_chars.append("\x1b[0m")
    # Print the entire frame in one go
    print("".join(frame_chars), end='')

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
