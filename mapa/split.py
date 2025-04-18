from PIL import Image
import os

# Load your big image
input_image = Image.open("mapa.png")
width, height = input_image.size

# Define tile size
tile_size = 508
output_dir = "tiles"
os.makedirs(output_dir, exist_ok=True)

# Loop to crop tiles
count = 0
for y in range(0, height, tile_size):
    for x in range(0, width, tile_size):
        # Crop with boundary check to avoid out-of-bounds
        box = (x, y, min(x + tile_size, width), min(y + tile_size, height))
        tile = input_image.crop(box)
        
        # Save the tile
        tile.save(f"{output_dir}/chunk_{y//tile_size}_{x//tile_size}.png")
        count += 1

print(f"Done. {count} tiles saved in '{output_dir}/'")

