# 🍌 Spinning Banana

An Ultra-Optimized Banana ASCII art animation, evolved from the famous 2006 Andrew Sloan spinning donut code.

## optimizations

This project implements "Senior Engineer" level optimizations for maximum scalar performance on standard CPUs:

1.  **Fixed-Point Arithmetic (16.16):** Replaces slow floating-point math with high-speed integer operations in the hot loop.
2.  **Bounding Box Rendering:** Calculates the exact screen area the banana occupies per frame and skips rendering ~85% of empty pixels.
3.  **Outer Loop Strength Reduction:** Eliminates multiplication entirely from the nested loops, relying on pure addition (`+=`) for coordinate mapping.
4.  **4x Loop Unrolling:** Manually unrolled inner loops reduce CPU branch prediction overhead.
5.  **Branchless Logic:** Uses bitwise integer casting for boundary checks to avoid pipeline stalls.
6.  **Cache Locality:** Flattened 1D memory arrays for maximum cache hit rates.

## Requirements

- **Python**: Python 3.x (Standard Library)
- **C**: GCC or Clang (MinGW on Windows)
- **C++**: G++ or Clang++ (MinGW on Windows)

## Build & Run

Run these commands from the project root:

### Python
No compilation needed. Optimized with buffered I/O.
```bash
python banana.py
```

### C (Recommended)
Compiles with `-O3` to enable loop vectorization.
```bash
gcc banana/banana.c -lm -O3 -o banana/banana_c
./banana/banana_c
```

### C++
Compiles with `-O3` for maximum speed.
```bash
g++ banana/banana.cpp -O3 -o banana/banana_cpp
./banana/banana_cpp
```

## Controls
Press `Ctrl+C` to stop the animation gracefully.

## Math Details

**Inverse Mapping & Strength Reduction**
Instead of calculating `x * cos(A)` for every pixel, we calculate the starting coordinate for the bounding box corner and simply add the pre-calculated slopes (sine/cosine values) as we traverse the grid.

```c
// Concept
row_start += slope_y;
pixel_coord += slope_x;
```

This reduces the per-pixel cost to just a few additions and bitwise shifts.