from PIL import Image
import struct
import random
import sys

def read_string(file):
    length = file.read(1)
    if len(length) != 1:
        sys.stderr.write(f"Failed reading string length in file\n")
        sys.exit(1)

    length = struct.unpack('B', length)[0]
    string_data = file.read(length)
    if len(string_data) != length:
        sys.stderr.write(f"Failed reading string in file\n")
        sys.exit(1)

    return string_data.decode('utf-8')

def read_number(file):
    number_data = file.read(4)
    if len(number_data) != 4:
        sys.stderr.write(f"Failed reading number in file\n")
        sys.exit(1)

    return struct.unpack('i', number_data)[0]

def read_compressed_number(file):
    # return read_number(file)
    num = 0
    shift = 0

    while True:
        byte = file.read(1)
        if len(byte) != 1:
            sys.stderr.write(f"Unexpected end of file while reading compressed number\n")
            sys.exit(1)

        byte = ord(byte)
        num |= (byte & 0x7F) << shift
        if not (byte & 0x80):
            break
        shift += 7

    return num

# char* read_string(FILE* in) {
#     uint8_t length;
#     if (fread(&length, sizeof(uint8_t), 1, in) != 1) {
#         fprintf(stderr, "Failed reading string length in file: %d\n", errno);
#         exit(EXIT_FAILURE);
#     }

#     char* str = (char*)malloc(length + 1);
#     if (fread(str, sizeof(char), length, in) != length) {
#         fprintf(stderr, "Failed reading string in file: %d\n", errno);
#         exit(EXIT_FAILURE);
#     }
#     str[length] = '\0';
#     return str;
# }

# int32_t read_number(FILE* in) {
#     int32_t num;
#     if (fread(&num, sizeof(int32_t), 1, in) != 1) {
#         fprintf(stderr, "Failed reading number in file: %d\n", errno);
#         exit(EXIT_FAILURE);
#     }
#     return num;
# }

# int32_t read_compressed_number(FILE* in) {
#     int32_t num = 0;
#     int shift = 0;
#     int byte;

#     while (1) {
#         byte = fgetc(in);
#         if (byte == EOF) {
#             if (feof(in)) {
#                 fprintf(stderr, "Unexpected end of file while reading compressed number\n");
#                 exit(EXIT_FAILURE);
#             } else {
#                 perror("Failed to read compressed number from file");
#                 exit(EXIT_FAILURE);
#             }
#         }

#         num |= (byte & 0x7F) << shift;
#         if (!(byte & 0x80)) {
#             break;
#         }
#         shift += 7;
#     }

#     return num;
# }

# void world_load(world* w, FILE* in) {
#     int width = read_number(in);
#     int height = read_number(in);

#     if (width != WORLD_WIDTH || height != WORLD_HEIGHT) {
#         fprintf(stderr, "World dimensions do not match!\n");
#         return;
#     }

#     int numBlocks = read_number(in);

#     struct { int key; char* value; }* loadblocks = NULL;
#     for (int i = 0; i < numBlocks; i++) {
#         char* str = read_string(in);
#         int id = read_number(in);

#         hmput(loadblocks, id, str);
#     }

#     int i = 0;
#     while (i < WORLD_WIDTH * WORLD_HEIGHT) {
#         int num = read_compressed_number(in);
#         int b = read_compressed_number(in);
#         int block = b & 0xff;
#         int data = b >> 8;
#         char* key = hmget(loadblocks, block);
#         block = shgeti(blocks, key) | data << 8;

#         for (int j = 0; j < num; j++) {
#             int x = i % WORLD_WIDTH;
#             int y = i / WORLD_WIDTH;

#             w->blocks[x][y] = block;

#             i++;
#         }
#     }

#     for (int i = 0; i < numBlocks; i++) {
#         free(loadblocks[i].value);
#     }

#     hmfree(loadblocks);
# }

with open("worlds/save.bin", "rb") as file:
    width = read_number(file)
    height = read_number(file)
    
    out = Image.new("RGB", (2000, 500))

    numBlocks = read_number(file)
    for i in range(numBlocks):
        read_string(file)
        read_number(file)
    
    i = 0
    while i < width * height:
        num = read_compressed_number(file)
        b = read_compressed_number(file)
        if b != 0:
            col = (
                random.randint(0, 255),
                random.randint(0, 255),
                random.randint(0, 255)
            )
            for j in range(num):
                x = i % width
                y = height - i // width - 1
                out.putpixel((x, y), col)
                i += 1
        else:
            i += num
    
    out.save("test.png")