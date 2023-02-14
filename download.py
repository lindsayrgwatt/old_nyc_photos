import csv
import requests
import shutil

file_path = '/Users/lindsayrgwatt/apps/nyc_photos/raw_images/'
url_append = '/datastream/OBJ/view'
    
urls = []

with open('urls.csv') as csv_file:
    url_reader = csv.reader(csv_file)
    for row in url_reader:
        url = row[0]
        #print(url)
        urls.append(url)

for url in urls:
    # URL format is similar to: http://dcmny.org/islandora/object/nyhs%3A826
    # Save image name as everything to right of '%'
    file_name = url.split('%')[1]
    #print(file_name)

    res = requests.get(url+url_append, stream = True)

    if res.status_code == 200:
        with open(file_path + file_name+'.jpg','wb') as f:
            shutil.copyfileobj(res.raw, f)
            print('Image sucessfully Downloaded: ',file_name)
    else:
        print('Image Couldn\'t be retrieved')


