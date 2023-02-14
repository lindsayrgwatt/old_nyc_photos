import os
import shutil

from PIL import Image
from pathlib import Path

INKPLATE_WIDTH = 825
INKPLATE_HEIGHT = 1200

# Goal
# 1. Open every image
# 2. Change width to 825
# 3. If taller than 1200, take middle 1200 pixels otherwise leave

def list_images():
    paths = []
    p = Path('.')
    q = p / 'categorized_images'
    subs = [x for x in q.iterdir() if x.is_dir()]
    for sub in subs:
        sub_dir = q / sub.name
        files = [x for x in sub_dir.iterdir() if not x.is_dir()]
        for file in files:
            paths.append(file)

    return paths


def resize(image_path, max_width, max_height):
# Will keep original aspect ratio and not resize beyond specified heights
    size = (max_width, max_height)

    outfile = os.path.splitext(image_path)[0] + "_resized.jpg"
    try:
        with Image.open(image_path) as im:
            im.thumbnail(size)
            im.save(outfile, "JPEG")
    except:
        print("cannot create thumbnail for", image_path)

    return True

def add_border(image_path, max_width, max_height):
    return true

def resize_all():
    paths = list_images()
    for path in paths:
        resize(path, INKPLATE_WIDTH, INKPLATE_HEIGHT)
    return True

def reshape(image_path, max_width=INKPLATE_WIDTH, max_height=INKPLATE_HEIGHT):
    temp = Image.new(
            mode="RGB",
            size = (max_width, max_height),
            color = (4, 4, 4)
        )

    photo = Image.open(image_path)
    width, height = photo.size

    delta = int((max_height - height)/2)

    Image.Image.paste(temp, photo, (0, delta))

    outfile = os.path.splitext(image_path)[0] + "_reshaped.jpg"
    try:
        temp.save(outfile, "JPEG")
    except:
        print("cannot save file", image_path)

    return True

def reshape_all(directory):
    paths = []
    p = Path('.')
    q = p / directory
    files = [x for x in q.iterdir() if not x.is_dir()]
    for file in files:
        paths.append(file)

    print(paths)

    for path in paths:
        reshape(path)
    return True

def rename(in_dir, out_dir):
    paths = []
    p = Path('.')
    q = p / in_dir
    files = [x for x in q.iterdir() if not x.is_dir()]
    counter = 1
    for file in files:
        start_path = file
        end_path = out_dir + '/' + str(counter) + '.jpg'
        shutil.move(start_path, end_path)
        counter += 1


