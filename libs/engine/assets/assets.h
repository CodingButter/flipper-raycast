// Engine assets — type definitions the engine uses to consume game-supplied
// content. The engine knows what a "Texture" or "SpriteSheet" looks like;
// the *actual* asset data (brick pixels, coin pixels, etc.) lives in
// game/assets/ and gets bundled by the game's own registry.
//
// Pixel encoding convention used by the renderers:
//   • Texture pixels:     1 = drawn (black on screen), 0 = transparent
//   • SpriteSheet pixels: 0 = transparent, 1 = black, 2 = white-punch-through
//
// Both formats are flat row-major byte arrays — one byte per pixel — so
// sampling is a single indexed read with no bit unpacking.
#pragma once

#include <stdint.h>

namespace Engine {

// Static wall / surface texture.
struct Texture {
    int w;
    int h;
    const uint8_t* data;   // w * h bytes, 1=drawn, 0=transparent
};

// Multi-frame animation. Frames are stored back-to-back:
//   data[f * (frame_h * frame_w) + j * frame_w + i]
struct SpriteSheet {
    int frame_w;
    int frame_h;
    int frame_count;
    const uint8_t* data;   // (frame_count * frame_w * frame_h) bytes, 0/1/2
};

} // namespace Engine
