# Font Conversion Process

This document explains how we converted the Taipan font from `taipan-font.png` to the C code in `font.h` for use with the Apple II TGI (Tiny Graphics Interface) library.

## Overview

The goal was to convert a bitmap font image into a C header file that can be used to render custom text on the Apple II. The process involves:

1. **Extracting** individual character glyphs from a source PNG image
2. **Downscaling** them from 14×16 pixels to 7×8 pixels (Apple II standard)
3. **Converting** to binary representation
4. **Generating** C code arrays

## Source Font Format

### Image Layout: `taipan-font.png`

- **Dimensions**: 258×110 pixels
- **Grid**: 6 rows × 16 columns = 96 characters
- **Character mapping**: ASCII 32 (space) through ASCII 127 (DEL)
- **Character order**: Left-to-right, top-to-bottom

```
Row 1: <space> ! " # $ % & ' ( ) * + , - . /
Row 2: 0 1 2 3 4 5 6 7 8 9 : ; < = > ?
Row 3: @ A B C D E F G H I J K L M N O
Row 4: P Q R S T U V W X Y Z [ \ ] ^ _
Row 5: ` a b c d e f g h i j k l m n o
Row 6: p q r s t u v w x y z { | } ~ <block>
```

### Cell Structure

Each character cell in the source image:
- **Cell size**: 14×16 pixels
- **Border**: 2 pixels of gray between each cell
- **Total cell spacing**: 16 pixels wide × 18 pixels tall (including borders)
- **Glyph color**: White pixels on black background
- **Border color**: Gray (#808080 or similar)

### Coordinate Calculation

To extract a character at grid position (row, col):
```
x_start = 2 + col × (14 + 2)  // border + column * (width + border)
y_start = 2 + row × (16 + 2)  // border + row * (height + border)
```

## Target Font Format

### Apple II Font Specifications

- **Dimensions**: 7×8 pixels (standard Apple II font size)
- **Storage**: Array of 96 characters, each with 8 bytes (one per row)
- **Bit encoding**: Each byte represents one row, 7 bits used (bits 0-6)
- **Bit order**: MSB (bit 6) is leftmost pixel, LSB (bit 0) is rightmost

### C Structure

```c
static const unsigned char font_data[96][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // space (ASCII 32)
    {0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x08, 0x00}, // ! (ASCII 33)
    // ... 94 more characters
};
```

## Downscaling Method: 2:1 Ratio

The Taipan font is 14×16 pixels, but Apple II fonts are 7×8 pixels. We use a simple 2:1 downscaling:

### Algorithm

For each target pixel at position (tx, ty):
1. Map to source 2×2 block: `(sx, sy) = (tx × 2, ty × 2)`
2. Check all 4 pixels in the 2×2 block
3. If **any** pixel is ON, set target pixel to ON
4. Otherwise, set target pixel to OFF

### Why This Works

- **14 ÷ 2 = 7** (width)
- **16 ÷ 2 = 8** (height)
- This preserves the distinctive blocky, retro style of the Taipan font
- Thin lines (1-2 pixels wide) are preserved
- The "OR" logic ensures no detail is lost

### Visual Example

Before (14×16):
```
......######.
.##...........
#.#...........
#.#...........
.#....#.......
..####........
..............
```

After (7×8):
```
...###.
.##....
#.#....
#.#....
.#....#
..####.
.......
```

## Conversion Process

### Step 1: Extract Glyphs

The Python script `extract_font.py` reads the PNG and extracts each character:

```python
def extract_glyph(img, row, col, cell_width=14, cell_height=16, border=2):
    x_start = border + col * (cell_width + border)
    y_start = border + row * (cell_height + border)

    glyph_pixels = []
    for y in range(cell_height):
        row_pixels = []
        for x in range(cell_width):
            pixel = img.getpixel((x_start + x, y_start + y))
            row_pixels.append(1 if is_pixel_on(pixel) else 0)
        glyph_pixels.append(row_pixels)

    return glyph_pixels
```

### Step 2: Downscale 2:1

```python
def downscale_glyph_2to1(glyph_pixels):
    downscaled = []
    for ty in range(8):  # target height
        row = []
        for tx in range(7):  # target width
            # Get the 2x2 source block
            src_y = ty * 2
            src_x = tx * 2

            # Check if any pixel in the 2x2 block is on
            pixel_on = False
            for dy in range(2):
                for dx in range(2):
                    if glyph_pixels[src_y + dy][src_x + dx]:
                        pixel_on = True
                        break

            row.append(1 if pixel_on else 0)
        downscaled.append(row)

    return downscaled
```

### Step 3: Convert to Bytes

Each row of 7 pixels becomes a byte:

```python
def glyph_to_bytes(glyph_pixels):
    bytes_data = []
    for row in glyph_pixels:
        byte_val = 0
        for i, pixel in enumerate(row):
            if pixel:
                byte_val |= (1 << (len(row) - 1 - i))  # MSB first
        bytes_data.append(byte_val)
    return bytes_data
```

Example: A row with pattern `#..##.#` (where # = on, . = off)
- Binary: `1001101`
- Byte value: `0x4D` (77 decimal)

### Step 4: Generate C Code

The script outputs C array declarations:

```python
for idx, glyph in enumerate(all_glyphs):
    byte_data = glyph_to_bytes(glyph)
    hex_values = ', '.join(f'0x{b:02X}' for b in byte_data)
    char_repr = CHAR_MAP[idx]
    print(f"    {{{hex_values}}}, // {char_repr}")
```

## Using the Extraction Script

### Prerequisites

- Python 3.x
- Pillow library: `pip install Pillow`

### Running the Script

```bash
python extract_font.py > font_output.txt
```

The script will:
1. Load `taipan-font.png`
2. Extract all 96 characters
3. Downscale each to 7×8
4. Output complete C code for `font.h`
5. Show example visualizations of 'A' and 'T'

### Script Output

The script generates:
- Font data array (`font_data[96][8]`)
- Helper function: `get_font_data(char c)`
- Drawing function: `draw_char(int x, int y, char c)`
- Text drawing: `draw_custom_text(int x, int y, const char* text)`

## Integration with Apple II TGI

### Font Drawing Functions

The generated `font.h` includes functions to render text:

```c
void draw_char(int x, int y, char c) {
    const unsigned char* data = get_font_data(c);
    int row, col;
    for (row = 0; row < 8; ++row) {
        unsigned char line = data[row];
        for (col = 0; col < 7; ++col) {
            if (line & (1 << (6 - col))) {
                tgi_setpixel(x + col, y + row);
            }
        }
    }
}

void draw_custom_text(int x, int y, const char* text) {
    while (*text) {
        draw_char(x, y, *text);
        x += 8; // 7 pixels + 1 space
        text++;
    }
}
```

### Usage Example

```c
#include <tgi.h>
#include "font.h"

int main(void) {
    tgi_install(tgi_static_stddrv);
    tgi_init();

    tgi_setcolor(TGI_COLOR_WHITE);
    draw_custom_text(10, 20, "Welcome to the World of Taipan!");

    // ... rest of your code
}
```

## Adapting for Other Fonts

To convert a different font image, adjust these parameters:

### 1. Source Image Layout

In `extract_font.py`, modify:
- `cell_width` and `cell_height` - dimensions of each character cell
- `border` - pixels between cells
- Grid dimensions (currently 6×16)

### 2. Target Dimensions

Change `target_width` and `target_height` in the `downscale_glyph_2to1()` function:
```python
target_width = 7   # Change this
target_height = 8  # Change this
```

Update the downscaling ratio calculation:
```python
src_y = ty * (src_height // target_height)
src_x = tx * (src_width // target_width)
```

### 3. Character Mapping

Update the `CHAR_MAP` array to match your font's character order:
```python
CHAR_MAP = [
    ' ', '!', '"', '#', ... # Your character order
]
```

### 4. Different Downscaling Methods

For non-2:1 ratios, use region averaging instead:

```python
def downscale_glyph_average(glyph_pixels, target_width, target_height):
    src_height = len(glyph_pixels)
    src_width = len(glyph_pixels[0])

    downscaled = []
    for ty in range(target_height):
        row = []
        for tx in range(target_width):
            # Calculate source region
            src_y_start = int(ty * src_height / target_height)
            src_y_end = int((ty + 1) * src_height / target_height)
            src_x_start = int(tx * src_width / target_width)
            src_x_end = int((tx + 1) * src_width / target_width)

            # Count pixels in the region
            on_pixels = 0
            total_pixels = 0
            for sy in range(src_y_start, src_y_end):
                for sx in range(src_x_start, src_x_end):
                    total_pixels += 1
                    if glyph_pixels[sy][sx]:
                        on_pixels += 1

            # Threshold (adjust as needed)
            threshold = 0.4  # 40% of pixels must be on
            pixel_on = (on_pixels / total_pixels) > threshold
            row.append(1 if pixel_on else 0)

        downscaled.append(row)

    return downscaled
```

### 5. Pixel Detection Threshold

Adjust the `is_pixel_on()` threshold for different image formats:

```python
def is_pixel_on(pixel):
    if isinstance(pixel, tuple):
        r, g, b = pixel[0], pixel[1], pixel[2]
        return (r + g + b) > 384  # Adjust this threshold
    else:
        return pixel > 128  # For grayscale
```

## Tips and Best Practices

### Image Preparation

1. **Consistent cell sizes**: Ensure all character cells are exactly the same size
2. **Clear borders**: Use a distinct color for borders (gray works well)
3. **High contrast**: Use pure white on black for best results
4. **Grid alignment**: Make sure the grid is perfectly aligned

### Font Design

1. **7×8 is the sweet spot** for Apple II - readable but compact
2. **Keep it simple** - thin lines (1-2 pixels) work best at small sizes
3. **Test on hardware** or accurate emulator (like MicroM8)
4. **Consider readability** - some fonts look great large but blur at 7×8

### Downscaling Considerations

1. **2:1 ratio** is ideal when source is exactly double the target
2. **Use OR logic** (any pixel on → target on) to preserve thin lines
3. **Use averaging** with threshold for non-integer ratios
4. **Test the letter 'i'** - good indicator of thin line preservation
5. **Check 'W' and 'M'** - tests wide character handling

### Debugging

1. **Visualize in ASCII** - the script's `visualize_glyph()` helps spot issues
2. **Test individual characters** first before generating full set
3. **Compare on-screen** with source image side-by-side
4. **Check bit order** - MSB vs LSB can flip characters horizontally

## References

- [Apple II HGR Font Tutorial](https://github.com/Michaelangel007/apple2_hgr_font_tutorial) - Detailed explanation of Apple II font rendering
- [cc65 TGI Documentation](https://cc65.github.io/doc/funcref.html#tgi) - TGI library reference
- Original Taipan font from the classic Apple II game

## Files

- `taipan-font.png` - Source font image (14×16 per character)
- `extract_font.py` - Python script to convert PNG to C code
- `font.h` - Generated C header with font data and drawing functions
- `T-BEFORE.PNG` - Example: letter 'T' at 14×16 pixels
- `T-AFTER.PNG` - Example: letter 'T' at 7×8 pixels
- `FONT.md` - This documentation

## License

The conversion process and tools are provided for educational purposes. The Taipan font belongs to its original creators.
