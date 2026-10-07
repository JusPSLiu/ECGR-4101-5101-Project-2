import os
import sys
from PIL import Image

def gif_to_bmps(gif_path):
    # Create the output directory if it doesn't exist
    output_dir = f"{os.path.basename(gif_path).split('.')[0]}" + "-frames"

    os.makedirs(output_dir, exist_ok=True)
    
    with Image.open(gif_path) as im:
        frame_num = 0
        while True:
            # Format the filename (e.g., frame_000.bmp, frame_001.bmp)
            output_filename = os.path.join(output_dir, f"frame_{frame_num:03d}.bmp")
            
            # Convert to RGB to discard GIF palette constraints if needed
            bmp_frame = im.convert("RGB")
            bmp_frame.save(output_filename, "BMP")
            print(f"Saved: {output_filename}")
            
            frame_num += 1
            try:
                # Move to the next frame of the GIF
                im.seek(im.tell() + 1)
            except EOFError:
                # End of frames
                break


if __name__ == "__main__":
    # Usage: python3 convert_gif_2_bmp "/path/to/image.gif"

    arguments = sys.argv[1:]

    try:
        img_path = arguments[0]
        
    except Exception as e:
        # print(e)
        print("Usage: python3 convert_gif_2_bmp \"/path/to/image.gif\"")
        exit()

    gif_to_bmps(img_path)

# for file in "$PWD"/celebi-shiny-frames/*; do python3 resize_image.py "$file" 100; done