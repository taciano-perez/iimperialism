# Drawing Pictures on Apple II HGR Screen

This guide explains how to convert bitmap images to Apple II HGR (High Resolution Graphics) format and display them using cc65.

## Overview

The process involves three main steps:
1. Convert a bitmap image to Apple II HGR RAG (image fragment) format using `bmp2dhr`
2. Convert the RAG binary file to a C byte array using `xxd`
3. Write C code to draw the image on the HGR screen

## Step 1: Create Your Bitmap

Create a bitmap image (`.bmp` file) with your desired content. For example, `wiseman.bmp` is a small sprite image.

**Important sizing notes:**
- Apple II HGR screen is 280×192 pixels
- For small sprites/fragments, keep dimensions modest (e.g., 28×28 pixels)
- HGR pixels are not square - they're approximately 2:1 width:height ratio
- HGR has color artifacts - adjacent pixels create colors (violet, orange, green, blue, white, black)
- bitmap must use 8 bit color depth

## Step 2: Convert to HGR RAG Format

Use the `bmp2dhr` tool (also called `b2d`) to convert your bitmap to Apple II format:

```bash
b2d wiseman.bmp H F N
```

### Flag Explanations:

- **H** - HGR (High Resolution Graphics) mode (280×192, 6 colors)
- **F** - Fragment/Sprite output (creates a RAG file instead of full-screen image)
- **N** - (Exact meaning varies by bmp2dhr version, typically relates to color processing)

**DO NOT use the "S" flag** unless you specifically need pixel scaling. The "S" flag can incorrectly scale down small images, resulting in missing rows.

### Output:

This creates `WISEMAN.RAG` - a binary file containing:
- 2-byte header: `[width_in_bytes, total_data_bytes_or_height]`
- Image data: Each row is encoded as multiple bytes

## Step 3: Convert RAG to C Byte Array

Use `xxd` to convert the binary RAG file to a C-compatible byte array:

```bash
xxd -i WISEMAN.RAG > temp_wiseman.h
```

This creates a temporary header file with content like:
```c
unsigned char WISEMAN_RAG[] = {
  0x04, 0x1c, 0x80, 0xd0, 0x82, 0x80, ...
};
```

### Adding to pictures.h:

1. **Copy the byte array data** from the temporary file into `pictures.h`
2. **Rename the array** to follow the naming convention (e.g., `WISEMAN_DATA`)
3. **Make it static const** for better optimization:

```c
/* Wiseman portrait - 28x28 pixels (4 bytes × 28 rows) */
/* Generated from wiseman.bmp using: b2d H F N wiseman.bmp */
static const unsigned char WISEMAN_DATA[] = {
  0x04, 0x1c, 0x80, 0xd0, 0x82, 0x80, ...
};
```

4. **Delete the temporary file** after copying the data

**Note:** You don't need to define WIDTH and HEIGHT constants - the drawing function will read these values directly from each picture's header bytes for maximum flexibility.

## Step 4: Organizing Multiple Pictures

For projects with multiple images, organize them in a single `pictures.h` file:

### 4.1: Add Individual Picture Data Arrays

For each picture, create a separate data array:

```c
/* Wiseman portrait */
static const unsigned char WISEMAN_DATA[] = {
  0x04, 0x1c, 0x80, 0xd0, 0x82, 0x80, /* ... data ... */
};

/* Another picture */
static const unsigned char SOLDIER_DATA[] = {
  0x04, 0x1c, 0xAA, 0xBB, 0xCC, 0xDD, /* ... data ... */
};
```

### 4.2: Create a Picture Index Array

Create an array of pointers to all pictures:

```c
static const unsigned char* PICTURES_DATA[] = {
    WISEMAN_DATA,   // Index 0
    SOLDIER_DATA,   // Index 1
    /* Add more pictures here */
};
```

### 4.3: Define Picture Index Constants

Create named constants for easy reference:

```c
#define WISEMAN_PORTRAIT 0
#define SOLDIER_PORTRAIT 1
/* Add more picture indices here */
```

This approach allows you to:
- Add new pictures without modifying existing code
- Reference pictures by meaningful names instead of magic numbers
- Keep all picture data organized in one place

## Step 5: Understanding the Data Format

The RAG file format (with "H F N" flags) contains:

### Header (2 bytes):
- **Byte 0:** Width encoding (often 4 for images ~28 pixels wide)
- **Byte 1:** Height or data size indicator (e.g., 0x1c = 28 decimal)

### Data Structure:
Each row of the image is encoded as **4 bytes**, regardless of the actual pixel width. This is part of the Fragment format's structure.

**Example:** For a 28-pixel-wide × 28-pixel-tall image:
- Actual display width: 28 pixels = 4 bytes in HGR (7 pixels per byte)
- Rows: 28
- Total data: 28 rows × 4 bytes = 112 bytes
- Total file: 2 (header) + 112 (data) = 114 bytes

## Step 6: Create the HGR Row Lookup Table

Apple II HGR screen memory is not linearly mapped. Create a lookup table for row addresses:

```c
static const unsigned int HGR_ROWS[] = {
    0x2000, 0x2400, 0x2800, 0x2C00, 0x3000, 0x3400, 0x3800, 0x3C00,
    0x2080, 0x2480, 0x2880, 0x2C80, 0x3080, 0x3480, 0x3880, 0x3C80,
    0x2100, 0x2500, 0x2900, 0x2D00, 0x3100, 0x3500, 0x3900, 0x3D00,
    // ... (complete table has 192 entries for all screen rows)
};
```

This table maps logical row numbers (0-191) to their actual memory addresses.

## Step 7: Write the Drawing Function

Create a function to draw any picture at any screen position:

```c
void draw_picture(const unsigned char picture_index, const unsigned char x_byte, unsigned char y) {
    unsigned char i;

    // Get pointer to the selected picture data
    const unsigned char *picture_data = PICTURES_DATA[picture_index];

    // Read width and height from the picture header
    const unsigned char width = picture_data[0];   // Byte 0: width in bytes
    const unsigned char height = picture_data[1];  // Byte 1: height in rows

    // Start at index 2 to skip the header bytes
    const unsigned char *data_ptr = &picture_data[2];

    for (i = 0; i < height; i++) {
        // Calculate screen address for this row
        unsigned int screen_addr = HGR_ROWS[y + i] + x_byte;

        // Copy width bytes for this row
        memcpy((void*)screen_addr, data_ptr, width);

        // Advance to next row in data
        data_ptr += width;
    }
}
```

### Function Parameters:
- **picture_index:** Index into PICTURES_DATA array (use defined constants like WISEMAN_PORTRAIT)
- **x_byte:** Horizontal position in bytes (0-39 for HGR's 40-byte width)
- **y:** Vertical position in pixels (0-191)

### How It Works:
1. Uses picture_index to get the correct picture data from PICTURES_DATA array
2. Reads the width and height directly from the picture's header bytes
3. Skips the 2-byte header by starting at `&picture_data[2]`
4. For each row:
   - Calculates the screen memory address using `HGR_ROWS[y + i] + x_byte`
   - Copies `width` bytes from the data array to screen memory
   - Advances the data pointer by `width` to the next row

### Why This Approach?
- **Flexible:** Works with any number of pictures without code changes
- **Self-documenting:** Picture names are clear (WISEMAN_PORTRAIT vs. magic number 0)
- **Automatic:** Width and height are read from headers, so the function adapts to different image sizes
- **Scalable:** Easy to add new pictures - just add data, pointer, and constant

## Step 8: Using the Drawing Function

In your main program:

```c
#include <tgi.h>
#include "pictures.h"

int main(void) {
    // Initialize TGI (cc65's graphics library)
    tgi_install(a2_hi_tgi);
    tgi_init();

    // Draw the wiseman portrait at position (2 bytes, 40 pixels)
    draw_picture(WISEMAN_PORTRAIT, 2, 40);

    // Draw another picture (if you've added more)
    // draw_picture(SOLDIER_PORTRAIT, 10, 80);

    // Wait for keypress
    cgetc();

    // Clean up
    tgi_uninstall();
    return 0;
}
```

### Adding More Pictures:

To add a new picture:

1. Convert your BMP: `b2d H F N newpicture.bmp`
2. Convert to C array: `xxd -i NEWPICTURE.RAG > temp.h`
3. Copy the byte array data into `pictures.h` as `NEWPICTURE_DATA[]`
4. Add `NEWPICTURE_DATA` to the `PICTURES_DATA[]` array
5. Define a constant: `#define NEWPICTURE_INDEX 2` (or next available index)
6. Use it: `draw_picture(NEWPICTURE_INDEX, x, y);`

## Common Issues and Solutions

### Issue: Image displays only half-height
**Cause:** The "S" scaling flag in bmp2dhr incorrectly reduced the image height.
**Solution:** Reconvert without the "S" flag: `b2d H F N yourimage.bmp`

### Issue: Image appears doubled horizontally
**Cause:** Copying too many bytes per row (WISEMAN_WIDTH too large).
**Solution:** Verify the first header byte and adjust WISEMAN_WIDTH accordingly.

### Issue: Image has wrong colors
**Cause:** Copying wrong bytes from each row's data.
**Solution:** Ensure you're copying all WISEMAN_WIDTH bytes starting from the correct offset.

### Issue: Image appears as random lines
**Cause:** Incorrect row width or data pointer advancement.
**Solution:** Verify WISEMAN_WIDTH matches the first header byte, and that you're advancing data_ptr correctly.

## Technical Notes

### HGR Color Artifacts
HGR mode uses color artifacts based on bit patterns:
- Adjacent ON pixels create colors based on column position
- Even columns: Violet/Blue
- Odd columns: Orange/Green
- Two adjacent ON pixels: White
- OFF pixels: Black

### Memory Layout
HGR screen memory ($2000-$3FFF) is organized in a complex pattern:
- Not sequential by row
- Each 8-row group is interleaved
- Use the HGR_ROWS lookup table to find correct addresses

### Byte Array Format
The RAG format with "H F N" flags produces a compact format where:
- Each row is consistently encoded as WISEMAN_WIDTH bytes
- The format includes the raw pixel data that can be directly copied to HGR screen memory
- No additional decompression or decoding is needed

## For AI Agents

When helping users with Apple II HGR image display:

1. **Always check the bmp2dhr flags used** - the "S" flag can cause issues with small images
2. **Read dimensions from the RAG header, don't hardcode them:**
   - Byte 0 (RAG[0]): width in bytes per row
   - Byte 1 (RAG[1]): height in rows (may need hex-to-decimal conversion, e.g., 0x1c = 28)
3. **Make drawing functions flexible** - read width/height from header instead of using #define constants
4. **Each row in the RAG format is width bytes** - copy all of them, don't assume a fixed size
5. **Skip the first 2 bytes of the array** - they're the header, not image data
6. **Use memcpy to transfer data directly to screen memory** - it's the fastest method
7. **The HGR_ROWS lookup table is essential** - HGR memory is not linear
8. **Avoid over-engineering** - the RAG format with "H F N" flags is already optimized; just copy bytes directly

## References

- bmp2dhr tool: http://www.appleoldies.ca/bmp2dhr/
- Apple II HGR Technical Notes: http://www.apple2.org.za/gswv/a2zine/faqs/csa2faq.html
- cc65 compiler documentation: https://cc65.github.io/

