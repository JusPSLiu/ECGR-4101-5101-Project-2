from PIL import Image
import os
import sys

def resize_image(img_path, width, new_img_path):
    # 1. Open the original image
    img = Image.open(img_path)

    # 3. Calculate the proportional height to maintain the aspect ratio
    w_percent = width / float(img.size[0])
    target_height = int(float(img.size[1]) * float(w_percent))

    # 4. Resize using the high-quality LANCZOS filter
    resized_img = img.resize((width, target_height), Image.NEAREST)

    # 5. Save with maximum quality settings
    basename = os.path.basename(img_path).split(".")[0]
    new_img_path = os.path.join(new_img_path, f"{basename}-{width}x{target_height}.bmp")

    resized_img.save(new_img_path)

    print(f"Imaged resized: {new_img_path}")

if __name__ == "__main__":
    # "Usage: python3 resize_image \path/to/image"

    arguments = sys.argv[1:]

    try:
        img_path = arguments[0]
        width = int(arguments[1])
        
    except Exception as e:
        # print(e)
        print("Usage: python3 resize_image \"/path/to/image\"")
        exit()
    
    if os.path.isdir(img_path):
        parent_name = os.path.basename(img_path)
        save_path = f"{parent_name}-resized-{width}x{width}"
        
        if not os.path.exists(save_path):
            os.mkdir(save_path)

        for root, dir, files in os.walk(img_path):
            files = sorted(files)

            for file in files:
                file_path = os.path.join(root, file)
                resize_image(file_path, width, save_path)

    elif os.path.isfile(img_path):
        parent_name = os.path.basename(os.path.dirname(img_path))
        save_path = f"{parent_name}-resized-{width}x{width}"
        
        if not os.path.exists(save_path):
            os.mkdir(save_path)

        resize_image(img_path, width, save_path)
    