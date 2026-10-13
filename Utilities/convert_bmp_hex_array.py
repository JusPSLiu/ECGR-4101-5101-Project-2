from PIL import Image
import os
import sys

def image_to_2d_hex_array(image_path):
    # 1. Open the image and ensure it's in RGB mode
    img = Image.open(image_path).convert('RGB')
    width, height = img.size
    
    hex_matrix = []
    
    # 2. Iterate through each row and column (2D)
    for y in range(height):
        row = []
        for x in range(width):
            r, g, b = img.getpixel((x, y))
            
            # 3. Convert 24-bit RGB (8-8-8) to 16-bit RGB565 (5-6-5 bits = 2 bytes)
            # Red: 5 bits, Green: 6 bits, Blue: 5 bits
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            
            # 4. Format as a 2-byte (4-character) Hex string (e.g., 0xFFFF)
            hex_str = f"0x{rgb565:04X}"
            row.append(hex_str)
            
        hex_matrix.append(row)

    print(f"Converted Image: {image_path}")
        
    return hex_matrix

def print_c_style_array(hex_matrix, index=0, row_per_line=False):
    """Optional utility to print the matrix in a clean C-style array format."""
    result = ""

    result += f"const unsigned short image_array{index}[][%d] = " % len(hex_matrix[0])
    result += "{"
    # result += "\n"

    for index, row in enumerate(hex_matrix):
        result += "{" + ", ".join(row) + "}"

        if index != len(hex_matrix) - 1:
            result += ","

        if row_per_line:
            result += "\n"

    result += "};"
    
    return result


if __name__ == "__main__":
    # Usage: python3 convert_bmp_hex_array "/path/to/image.bmp"

    arguments = sys.argv[1:]

    try:
        img_path = arguments[0]
    except Exception as e:
        # print(e)
        print("Usage: python3 convert_bmp_hex_array \"/path/to/image.bmp\"")
        exit()

    final_str = ""

    if os.path.isdir(img_path):
        count = 0

        parent_name = os.path.basename(img_path)
        save_path = f"{parent_name}-c-arrays"
        
        if not os.path.exists(save_path):
            os.mkdir(save_path)

        basename = os.path.basename(img_path).split('.')[0]
        c_file_path = os.path.join(save_path, f"{basename}.c")

        with open(c_file_path, "w") as f:
            for root, dir, files in os.walk(img_path):
                files = sorted(files)

                for file in files:
                    file_path = os.path.join(root, file)

                    if not file.startswith("."):
                        hex_array = image_to_2d_hex_array(file_path)
                        final_str += print_c_style_array(hex_array, index=count) + "\n"
                        count += 1
            f.close()

    elif os.path.isfile(img_path):
        parent_name = os.path.basename(os.path.dirname(img_path))
        save_path = f"{parent_name}-c-arrays"
        
        if not os.path.exists(save_path):
            os.mkdir(save_path)

        basename = os.path.basename(img_path).split('.')[0]
        c_file_path = os.path.join(save_path, f"{basename}.c")

        hex_array = image_to_2d_hex_array(img_path)
        final_str = print_c_style_array(hex_array)


    with open(c_file_path, "w") as f:
        f.write(final_str)
        f.close()

# for file in "$PWD"/celebi-shiny-frames/*; do python3 convert_bmp_hex_array "$file"; done 