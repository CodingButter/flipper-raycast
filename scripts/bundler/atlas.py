"""Asset-bundler core. Pure Python — no filesystem, no PIL, no platform deps.

Designed to run identically in CPython (CLI use) and Pyodide (browser IDE).
The CLI wrapper (bundle.py) decodes PNGs via PIL and feeds raw pixels in;
the browser IDE would use Canvas/createImageBitmap to do the same.

Engine pixel encoding (the only contract that matters):
    0 = transparent  (sprite renders see-through)
    1 = black        (drawn black on the bitmap)
    2 = white        (drawn white — explicit highlight in sprites)
"""

# ─────────────────────────────────────────────────────────────────────
# Pixel classification — take a decoded image dict and return a flat
# bytearray of 0/1/2 values in row-major order.
#
# `decoded` shape (platform-agnostic):
#   {
#     "width":  int,
#     "height": int,
#     "mode":   'rgba' | 'rgb' | 'grayscale' | 'indexed',
#     "data":   bytes / bytearray / list[int],
#     "palette": (optional, for indexed mode) — currently unused, indices
#                are trusted as-is.
#   }
# ─────────────────────────────────────────────────────────────────────
def _classify_rgb(r, g, b, a=255):
    """Map a single RGB(A) color → engine byte (0/1/2)."""
    if a == 0:
        return 0
    # Magenta sentinel for "transparent" (255, 0, 255).
    if r > 200 and g < 80 and b > 200:
        return 0
    lum = (r * 299 + g * 587 + b * 114) // 1000
    if lum < 64:  return 1
    if lum > 192: return 2
    return 0


def classify_image(decoded):
    width  = decoded["width"]
    height = decoded["height"]
    mode   = decoded["mode"]
    data   = decoded["data"]
    n      = width * height
    out    = bytearray(n)

    if mode == "indexed":
        # Build a palette-index → engine-byte map by inspecting each
        # palette entry's actual RGB. This way the user can author with
        # ANY palette order; magenta/black/white in any slots map correctly.
        pal = decoded.get("palette") or b""
        entries = len(pal) // 3
        idx_map = bytearray(256)                  # default 0 (transparent)
        for i in range(min(entries, 256)):
            r = pal[i*3 + 0]
            g = pal[i*3 + 1]
            b = pal[i*3 + 2]
            idx_map[i] = _classify_rgb(r, g, b)
        for i in range(n):
            out[i] = idx_map[data[i]]
        return out

    if mode == "rgba":
        for i in range(n):
            out[i] = _classify_rgb(data[i*4], data[i*4+1], data[i*4+2], data[i*4+3])
        return out

    if mode == "rgb":
        for i in range(n):
            out[i] = _classify_rgb(data[i*3], data[i*3+1], data[i*3+2])
        return out

    if mode == "grayscale":
        for i in range(n):
            v = data[i]
            out[i] = 1 if v < 64 else (2 if v > 192 else 0)
        return out

    raise ValueError(f"unsupported decoded mode: {mode!r}")


# ─────────────────────────────────────────────────────────────────────
# Slice a multi-frame image into a contiguous byte stream — frame 0's
# pixels first, then frame 1's, etc. The engine expects this order.
# ─────────────────────────────────────────────────────────────────────
def pack_frames(classified, img_w, img_h, frame_w, frame_h, frame_count, layout):
    if frame_count == 1:
        if img_w != frame_w or img_h != frame_h:
            raise ValueError(
                f"image {img_w}x{img_h} doesn't match declared {frame_w}x{frame_h}")
        return classified

    layout = layout or "horizontal"
    if layout == "horizontal":
        if img_w != frame_w * frame_count or img_h != frame_h:
            raise ValueError(
                f"image {img_w}x{img_h} doesn't match "
                f"{frame_w}x{frame_h}x{frame_count} horizontal strip")
    elif layout == "vertical":
        if img_w != frame_w or img_h != frame_h * frame_count:
            raise ValueError(
                f"image {img_w}x{img_h} doesn't match "
                f"{frame_w}x{frame_h}x{frame_count} vertical strip")
    else:
        raise ValueError(f"unknown layout: {layout!r}")

    out = bytearray(frame_w * frame_h * frame_count)
    cur = 0
    for f in range(frame_count):
        for y in range(frame_h):
            for x in range(frame_w):
                if layout == "vertical":
                    sx, sy = x, f * frame_h + y
                else:
                    sx, sy = f * frame_w + x, y
                out[cur] = classified[sy * img_w + sx]
                cur += 1
    return out


# ─────────────────────────────────────────────────────────────────────
# Emit assets.cpp + assets.h text from the manifest + per-asset pixel
# data.
#
# `manifest` shape:
#   {
#     "assets": [
#       {
#         "id": "BRICK",
#         "kind": "texture" | "spriteSheet" | "atlas",
#         "w": int, "h": int,
#         "frames": int (default 1),
#         "layout": "horizontal" | "vertical" (default horizontal),
#         "wallIndex": int (optional — registers in WALL_TEXTURES),
#         "views": [                         (optional — extra SpriteSheet
#           { "id", "frameStart", "frames" }   aliases into same bytes)
#         ],
#       },
#       ...
#     ],
#     "output": { "cpp": "...", "h": "..." }
#   }
#
# `pixels`: dict { asset_id → { "width", "height", "classified": bytearray } }
#
# `kind: "atlas"` packs frames + emits ONLY the named views (no primary
# sheet for the id itself). Use this when one image holds several
# logically-distinct sprite sheets (e.g. directional skeleton).
# ─────────────────────────────────────────────────────────────────────
def pack_2bpp(unpacked):
    """Pack 1-byte-per-pixel (values 0..3) into 2-bits-per-pixel.
    4 pixels per byte; little-endian inside the byte (pixel 0 = bits 0-1).
    Pads to a whole byte if the pixel count isn't a multiple of 4."""
    n = len(unpacked)
    out = bytearray((n + 3) // 4)
    for i in range(n):
        out[i >> 2] |= (unpacked[i] & 0x3) << ((i & 3) << 1)
    return out


def build_atlas(manifest, pixels):
    chunks = []          # list of (asset_dict, packed_bytes, unpacked_pixel_count)
    offsets = {}         # asset_id → BYTE offset into ATLAS_DATA (packed)
    cursor = 0

    for a in manifest["assets"]:
        px = pixels.get(a["id"])
        if px is None:
            raise ValueError(f"missing pixel data for asset {a['id']!r}")
        frames = a.get("frames", 1)
        unpacked = pack_frames(px["classified"], px["width"], px["height"],
                               a["w"], a["h"], frames, a.get("layout"))
        expected_pixels = a["w"] * a["h"] * frames
        if len(unpacked) != expected_pixels:
            raise ValueError(
                f"asset {a['id']!r} produced {len(unpacked)} pixels, expected {expected_pixels}")
        packed = pack_2bpp(unpacked)
        chunks.append((a, packed, expected_pixels))
        offsets[a["id"]] = cursor
        cursor += len(packed)

    # ── assets.cpp ───────────────────────────────────────────────────
    cpp_lines = [
        "// AUTO-GENERATED by the FapForge bundler. DO NOT EDIT.",
        "// Re-export from the IDE's asset panel to regenerate.",
        "#include <stdint.h>",
        '#include "../../libs/engine/assets/assets.h"',
        "",
        "namespace Game { namespace Assets {",
        "",
        "extern const uint8_t ATLAS_DATA[] = {",
    ]
    for a, b, _ in chunks:
        cpp_lines.append(f"    // {a['id']} — offset {offsets[a['id']]}, "
                         f"{len(b)} packed bytes (2 bpp)")
        for i in range(0, len(b), 16):
            cpp_lines.append("    " + ",".join(str(x) for x in b[i:i+16]) + ",")
    cpp_lines.append("};")
    cpp_lines.append("")

    # `extern const` is required: C++ gives namespace-scope `const`
    # variables internal linkage by default, so without `extern` the
    # symbols are private to assets.cpp and other TUs can't see them.
    for a, _, _ in chunks:
        ofs = offsets[a["id"]]
        kind = a.get("kind", "spriteSheet")
        if kind == "texture":
            cpp_lines.append(
                f"extern const Engine::Texture     {a['id']:<20s} = "
                f"{{ {a['w']}, {a['h']}, ATLAS_DATA + {ofs} }};")
        elif kind == "spriteSheet":
            f = a.get("frames", 1)
            cpp_lines.append(
                f"extern const Engine::SpriteSheet {a['id']:<20s} = "
                f"{{ {a['w']}, {a['h']}, {f}, ATLAS_DATA + {ofs} }};")
        # `atlas` kind: don't emit a primary sheet; only views below.

        for v in a.get("views", []):
            # Frame offset converted from PIXELS to PACKED BYTES (÷4 since
            # we store 4 pixels per byte). Asset sizes here are multiples
            # of 4 so view frames stay byte-aligned.
            view_ofs = ofs + (v["frameStart"] * a["w"] * a["h"]) // 4
            cpp_lines.append(
                f"extern const Engine::SpriteSheet {v['id']:<20s} = "
                f"{{ {a['w']}, {a['h']}, {v['frames']}, ATLAS_DATA + {view_ofs} }};")
    cpp_lines.append("")
    cpp_lines.append("}} // namespace Game::Assets")

    # ── assets.h ─────────────────────────────────────────────────────
    h_lines = [
        "// AUTO-GENERATED by the FapForge bundler. DO NOT EDIT.",
        "#pragma once",
        "#include <stdint.h>",
        '#include "../../libs/engine/assets/assets.h"',
        "",
        "namespace Game { namespace Assets {",
        "",
    ]
    for a, _, _ in chunks:
        kind = a.get("kind", "spriteSheet")
        if kind == "texture":
            h_lines.append(f"extern const Engine::Texture     {a['id']};")
        elif kind == "spriteSheet":
            h_lines.append(f"extern const Engine::SpriteSheet {a['id']};")
        for v in a.get("views", []):
            h_lines.append(f"extern const Engine::SpriteSheet {v['id']};")
    h_lines.append("")

    walls = sorted(
        [a for a, _, _ in chunks if "wallIndex" in a],
        key=lambda a: a["wallIndex"])
    if walls:
        max_idx = walls[-1]["wallIndex"]
        h_lines.append("inline const Engine::Texture* const WALL_TEXTURES[] = {")
        h_lines.append("    nullptr,   // 0 = empty")
        for i in range(1, max_idx + 1):
            w = next((x for x in walls if x["wallIndex"] == i), None)
            if w: h_lines.append(f"    &{w['id']},  // {i}")
            else: h_lines.append(f"    nullptr,  // {i} = reserved")
        h_lines.append("};")
        h_lines.append(f"constexpr int WALL_TEXTURE_COUNT = {max_idx + 1};")
    h_lines.append("")
    h_lines.append("}} // namespace Game::Assets")

    return {
        "cpp": "\n".join(cpp_lines) + "\n",
        "h":   "\n".join(h_lines) + "\n",
        "total_bytes": cursor,
    }
