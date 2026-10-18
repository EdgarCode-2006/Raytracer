#!/usr/bin/env python3
"""Generates the textures of the scrolling space background (.png): a nebula and two star layers.

The ship models are made in Blender by the scripts of tools/models/.

Run from the repository root: python3 tools/generate_space_textures.py
"""

import os
import struct
import zlib

import numpy as np

TEXTURES_DIR = "assets/game/textures"


# Tileable horizontally and vertically: the background scrolls forever.

def write_png(path, rgb):
    height, width, _ = rgb.shape
    raw = b"".join(b"\x00" + rgb[y].tobytes() for y in range(height))

    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)))
        f.write(chunk(b"IDAT", zlib.compress(raw, 9)))
        f.write(chunk(b"IEND", b""))


def periodic_noise(width, height, cells_x, cells_y, rng):
    """Value noise that wraps around: the lattice repeats every cells_x by cells_y."""
    lattice = rng.random((cells_y, cells_x))
    x = np.arange(width) * cells_x / width
    y = np.arange(height) * cells_y / height
    x0, y0 = np.floor(x).astype(int), np.floor(y).astype(int)
    fx, fy = x - x0, y - y0
    fx, fy = fx * fx * (3 - 2 * fx), fy * fy * (3 - 2 * fy)
    x1, y1 = (x0 + 1) % cells_x, (y0 + 1) % cells_y
    top = lattice[y0][:, x0] * (1 - fx) + lattice[y0][:, x1] * fx
    bottom = lattice[y1][:, x0] * (1 - fx) + lattice[y1][:, x1] * fx
    return top * (1 - fy)[:, None] + bottom * fy[:, None]


def fbm(width, height, rng, octaves=5):
    total = np.zeros((height, width))
    amplitude, weight = 1.0, 0.0
    for octave in range(octaves):
        cells = 4 * 2 ** octave
        total += periodic_noise(width, height, cells * 2, cells, rng) * amplitude
        weight += amplitude
        amplitude *= 0.5
    return total / weight


def add_stars(image, rng, count, brightness, glow):
    height, width, _ = image.shape
    for _ in range(count):
        x, y = rng.integers(0, width), rng.integers(0, height)
        b = brightness * rng.uniform(0.4, 1.0)
        tint = np.array([1.0, 1.0, 1.0]) - rng.uniform(0, 0.25) * np.array(rng.permutation([1.0, 0.5, 0.0]))
        image[y, x] += b * tint
        if glow and b > brightness * 0.75:
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                image[(y + dy) % height, (x + dx) % width] += b * 0.35 * tint


def nebula_layer(width, height, rng):
    clouds = np.clip((fbm(width, height, rng) - 0.45) * 2.2, 0, 1) ** 2
    tint = fbm(width, height, rng, octaves=3)
    purple = np.array([0.55, 0.16, 0.70])
    blue = np.array([0.08, 0.28, 0.62])
    color = clouds[..., None] * (purple * tint[..., None] + blue * (1 - tint[..., None]))
    color += np.array([0.01, 0.01, 0.025])  # space is not pure black
    add_stars(color, rng, 2500, 0.35, glow=False)
    return color


def star_layer(width, height, rng, count, brightness):
    image = np.zeros((height, width, 3))
    add_stars(image, rng, count, brightness, glow=True)
    return image


def to_rgb8(image):
    return (np.clip(image, 0, 1) * 255).astype(np.uint8)


def main():
    os.makedirs(TEXTURES_DIR, exist_ok=True)

    rng = np.random.default_rng(42)
    width, height = 1024, 512
    write_png(f"{TEXTURES_DIR}/space_nebula.png", to_rgb8(nebula_layer(width, height, rng)))
    write_png(f"{TEXTURES_DIR}/space_stars_far.png", to_rgb8(star_layer(width, height, rng, 900, 0.6)))
    write_png(f"{TEXTURES_DIR}/space_stars_near.png", to_rgb8(star_layer(width, height, rng, 160, 1.0)))
    print("Space textures written to assets/game/textures/")


if __name__ == "__main__":
    main()
