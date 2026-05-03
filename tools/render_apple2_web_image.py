#!/usr/bin/env python3
"""Render Apple II bitmap assets for web display.

Apple II HGR pixels are wider than they are tall, so a sprite intended for that
screen should be stretched horizontally with nearest-neighbor scaling when
shown on the web. This tool applies that aspect correction and writes PNGs.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Render Apple II bitmap assets for web display."
    )
    parser.add_argument("inputs", nargs="+", help="Input image paths")
    parser.add_argument(
        "--output-dir",
        default="assets/img/web",
        help="Directory for rendered PNG output (default: assets/img/web)",
    )
    parser.add_argument(
        "--x-scale",
        type=int,
        default=2,
        help="Horizontal nearest-neighbor scale factor (default: 2)",
    )
    parser.add_argument(
        "--y-scale",
        type=int,
        default=1,
        help="Vertical nearest-neighbor scale factor (default: 1)",
    )
    return parser.parse_args()


def render_image(input_path: Path, output_dir: Path, x_scale: int, y_scale: int) -> Path:
    image = Image.open(input_path).convert("RGBA")
    rendered = image.resize(
        (image.width * x_scale, image.height * y_scale),
        resample=Image.Resampling.NEAREST,
    )
    output_path = output_dir / f"{input_path.stem}.png"
    rendered.save(output_path)
    return output_path


def main() -> int:
    args = parse_args()
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    if args.x_scale < 1 or args.y_scale < 1:
        raise ValueError("Scale factors must be positive integers")

    for raw_path in args.inputs:
        input_path = Path(raw_path)
        render_image(input_path, output_dir, args.x_scale, args.y_scale)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
