#!/usr/bin/env python3
"""CLI wrapper around atlas.py — reads PNGs from disk via PIL, writes
assets.cpp + assets.h to the project tree.

In a Pyodide browser IDE you'd replace decode_image_from_file with
something that reads the user's uploaded File via the browser's image
APIs, then feeds the same shape into atlas.build_atlas(). Everything
in atlas.py is dependency-free and runs unchanged in Pyodide.

Usage:
    python3 scripts/bundler/bundle.py [scripts/bundler/manifest.json]
"""
import json
import os
import sys
from pathlib import Path

from PIL import Image

# Allow running from anywhere: import atlas.py from this directory.
sys.path.insert(0, str(Path(__file__).parent))
from atlas import classify_image, build_atlas


def decode_image_from_file(path):
    """Read an image off disk and convert to the platform-agnostic shape
    that atlas.classify_image() expects. Indexed PNGs are passed through
    as indexed; everything else becomes RGBA (PIL handles the conversion).
    """
    im = Image.open(path)
    # 1-bit BMPs ("mode 1") have an implicit 2-color palette. Promote to
    # "P" so we go down the indexed path with a real palette.
    if im.mode == "1":
        im = im.convert("P")
    if im.mode == "P":
        # Use im.getpalette() — it always returns clean RGB triplets
        # regardless of how the BMP stored them. im.palette.palette is
        # raw and varies (3 vs 4 bytes per entry depending on source).
        pal_list = im.getpalette() or []
        return {
            "width":   im.width,
            "height":  im.height,
            "mode":    "indexed",
            "data":    im.tobytes(),
            "palette": bytes(pal_list),
        }
    if im.mode != "RGBA":
        im = im.convert("RGBA")
    return {
        "width":  im.width,
        "height": im.height,
        "mode":   "rgba",
        "data":   im.tobytes(),
    }


def main():
    if len(sys.argv) > 1:
        manifest_path = Path(sys.argv[1]).resolve()
    else:
        manifest_path = Path(__file__).parent / "manifest.json"

    manifest_dir = manifest_path.parent
    manifest = json.loads(manifest_path.read_text())

    # Decode + classify every asset's source image.
    pixels = {}
    for a in manifest["assets"]:
        src = (manifest_dir / a["src"]).resolve()
        decoded = decode_image_from_file(src)
        pixels[a["id"]] = {
            "width":      decoded["width"],
            "height":     decoded["height"],
            "classified": classify_image(decoded),
        }
        print(f"  ✓ {a['id']:<22s} {decoded['width']}×{decoded['height']}  {a['src']}")

    result = build_atlas(manifest, pixels)

    # Resolve output paths relative to the project root (manifest is in
    # scripts/bundler/, project root is two levels up).
    project_root = manifest_dir.parent.parent
    cpp_path = (project_root / manifest["output"]["cpp"]).resolve()
    h_path   = (project_root / manifest["output"]["h"]  ).resolve()
    cpp_path.parent.mkdir(parents=True, exist_ok=True)
    h_path.parent.mkdir(parents=True, exist_ok=True)
    cpp_path.write_text(result["cpp"])
    h_path.write_text(result["h"])

    print()
    print(f"  wrote {cpp_path.relative_to(project_root)}  ({len(result['cpp'])} bytes)")
    print(f"  wrote {h_path.relative_to(project_root)}  ({len(result['h'])} bytes)")
    print(f"  atlas: {result['total_bytes']} bytes across {len(manifest['assets'])} assets")


if __name__ == "__main__":
    main()
