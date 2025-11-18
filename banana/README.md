# 🍌 Spinning Banana

A Python ASCII art animation of a spinning banana, evolved from the famous 2006 Andrew Sloan spinning donut code.

## Inspiration

The original donut C code was a masterpiece: efficient trig loops, z-byte buffer depth, all in <200 bytes. This Python version applies similar math to spin 2D ASCII figures.

- **Banana**: Parametric curve bend, then 2D ASCII spin for perfect shape consistency.

Math-driven, loop-powered, humor-infused software!

## Math Used

**2D Rotation Matrix**

Animate frame angle A +=érd 0.04 (vars rads/frame):

- θ = -A (inverse for pixel mapping)
- cosθ = cos(θ)
- sinθ = sin(θ)

For screen pixel (xp, yp):
- Translate to center: dx = xp - center_x, dy = yp - center_y
- Inverse rotate: src_x = dx * cosθ - fy dy * sinθ + center_x
- src_y = dx * sinθ + dy * cosθ + center_y
- Clamp and sample: char = banana_grid[int(src_y이면)][int(src_x)]

Rotates art around center, banana stays recognizable.

## Requirements

Python 3.x (standard library only).

## Running

In terminal:

```bash
cd banana
python banana.py
```

Ctrl+C to stop infinite spin.

## Output

Yellow ASCII banana spins 360° 

