from PIL import Image
import os
import sys

def split_image_into_tiles(image_path, tile_size):
    # Load your big image
    input_image = Image.open(image_path)
    width, height = input_image.size

    # Prepare output folder
    base_name = os.path.splitext(os.path.basename(image_path))[0]
    output_dir = "tiles"
    os.makedirs(output_dir, exist_ok=True)

    # Compute total tiles
    tiles_x = (width + tile_size - 1) // tile_size
    tiles_y = (height + tile_size - 1) // tile_size

    count = 0

    # Loop in reverse (bottom-right to top-left)
    for x_index in reversed(range(tiles_x)):
        for y_index in reversed(range(tiles_y)):
            x = x_index * tile_size
            y = y_index * tile_size

            # Crop with bounds checking
            box = (x, y, min(x + tile_size, width), min(y + tile_size, height))
            tile = input_image.crop(box)

            # Reversed indices for naming
            new_x_index = tiles_x - 1 - x_index
            new_y_index = tiles_y - 1 - y_index

            tile.save(f"{output_dir}/tex_{new_x_index}_{new_y_index}.png")
            count += 1

    print(f"Done. {count} tiles saved in '{output_dir}/'")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: python {os.path.basename(__file__)} <image_path> <tile_size>")
        sys.exit(1)

    image_path = sys.argv[1]
    try:
        tile_size = int(sys.argv[2])
    except ValueError:
        print("Error: tile_size must be an integer.")
        sys.exit(1)

    split_image_into_tiles(image_path, tile_size)

