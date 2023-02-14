import os
import shutil

from PIL import Image
from pathlib import Path

INKPLATE_WIDTH = 825
INKPLATE_HEIGHT = 1200

def get_all_images(directory):
    paths = []
    p = Path('.')
    q = p / directory
    files = [x for x in q.iterdir() if not x.is_dir()]
    for file in files:
        paths.append(file)

    return paths

# Resize all the images for Inkplate and add border
def resize_and_pad(image_path, max_width=INKPLATE_WIDTH, max_height=INKPLATE_HEIGHT):
# Will keep original aspect ratio and not resize beyond specified heights
    size = (max_width, max_height)
    
    try:
        photo = Image.open(image_path)
    except:
        print("cannot open file", image_path)
        return False
    photo.thumbnail(size)

    temp = Image.new(
            mode="RGB",
            size = (max_width, max_height),
            color = (4, 4, 4)
        )

    width, height = photo.size

    delta_x = 0
    delta_y = 0

    if width < max_width:
        delta_x = int((max_width - width)/2)

    if height < max_height:
        delta_y = int((max_height - height)/2)

    Image.Image.paste(temp, photo, (delta_x, delta_y))

    return temp

# Rename and move
def rename_and_save(photo, out_directory, counter):
    p = Path('.')
    q = p / out_directory
    
    out_path = str(q) + '/' + str(counter) + '.jpg'

    try:
        photo.save(out_path, "JPEG")
    except:
        print("cannot save file", out_path)
    return True

def convert(directory, out_directory):
    paths = get_all_images(directory)

    for counter, image_path in enumerate(paths):
        photo = resize_and_pad(image_path)
        if photo: # Can fail due to .ds_store or other non-image files
            rename_and_save(photo, out_directory, counter+1)

    return True
