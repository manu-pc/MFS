from PIL import Image
import os

# Load your big image
input_image = Image.open("cee.png")
width, height = input_image.size

# Define tile size
tile_size = 320
output_dir = "tiles"
os.makedirs(output_dir, exist_ok=True)

# Compute total tiles in each dimension
tiles_x = (width + tile_size - 1) // tile_size
tiles_y = (height + tile_size - 1) // tile_size

count = 0

# Loop in reverse (from bottom right to top left)
for x_index in reversed(range(tiles_x)):
    for y_index in reversed(range(tiles_y)):
        x = x_index * tile_size
        y = y_index * tile_size

        # Crop box with bounds checking
        box = (x, y, min(x + tile_size, width), min(y + tile_size, height))
        tile = input_image.crop(box)

        # Compute reversed tile indices for naming
        new_x_index = tiles_x - 1 - x_index
        new_y_index = tiles_y - 1 - y_index

        tile.save(f"{output_dir}/tex_{new_x_index}_{new_y_index}.png")
        count += 1

print(f"Done. {count} tiles saved in '{output_dir}/'")

