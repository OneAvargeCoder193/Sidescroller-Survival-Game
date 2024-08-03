from PIL import Image, ImageDraw
import random
import struct

# int i = 0;
# while (i < WORLD_WIDTH * WORLD_HEIGHT) {
#     int num = read_number(in);
#     int b = read_number(in);
#     int block = b & 0xff;
#     int data = b >> 8;
#     char* key = hmget(loadblocks, block);
#     block = shgeti(blocks, key) | data << 8;

#     for (int j = 0; j < num; j++) {
#         int x = i % WORLD_WIDTH;
#         int y = i / WORLD_WIDTH;

#         w->blocks[x][y] = block;

#         i++;
#     }
# }

def read_number(file):
    return struct.unpack("i", file.read(4))[0]

def read_string(file):
    len = ord(file.read(1))
    return file.read(len).decode()

def read_compressed_number(file):
    num = 0
    shift = 0
    while True:
        byte = file.read(1)
        if not byte:
            raise EOFError("Unexpected end of file while reading compressed number")
        byte = ord(byte)
        num |= (byte & 0x7F) << shift
        if byte & 0x80 == 0:
            break
        shift += 7
    return num

with open("save.bin", "rb") as file:
    width = read_number(file)
    height = read_number(file)

    img = Image.new("RGB", (width, height))
    draw = ImageDraw.Draw(img)

    numBlocks = read_number(file)

    for i in range(numBlocks):
        read_string(file)
        read_number(file)

    i = 0
    n = 0
    while i < width * height:
        n += 1
        num = read_number(file)
        data = read_compressed_number(file)
        # print(data & 0xff)

        color = (random.randrange(255), random.randrange(255), random.randrange(255))

        # print(i, i + num, width, height, width * height)

        for j in range(num):
            x = i % width
            y = i // width

            # print(x, y, i)

            if data != 0:
                img.putpixel((x, y), color)

            # print(x, y)

            i += 1
        
    print(n)

    img.save("out.png", "PNG")