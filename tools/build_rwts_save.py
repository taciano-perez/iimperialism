from __future__ import annotations

import argparse
from pathlib import Path


SLOT_COUNT = 5
CONTAINER_VERSION = 3
SAVE_HEADER_VERSION = 4
GAME_STATE_SIZE = 186
SAVE_HEADER_SIZE = 3
RECORD_SIZE = SAVE_HEADER_SIZE + GAME_STATE_SIZE
CONTAINER_SIZE = 4
PADDED_SIZE = 1024


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    output = Path(args.output)

    image = bytearray(PADDED_SIZE)
    image[0:4] = bytes((ord("I"), ord("S"), CONTAINER_VERSION, SLOT_COUNT))

    for slot in range(SLOT_COUNT):
        record_offset = CONTAINER_SIZE + slot * RECORD_SIZE
        image[record_offset : record_offset + 2] = b"\x00\x00"
        image[record_offset + 2] = SAVE_HEADER_VERSION

    output.write_bytes(image)


if __name__ == "__main__":
    main()
