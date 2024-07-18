from PIL import Image, ImageDraw

old = Image.open("olddirt.png")

img = Image.new("RGBA", (160, 4))

table = [
    32,
    16,
    17,
    18,
    19,
    33,
    24,
    0,
    1,
    2,
    3,
    28,
    25,
    4,
    5,
    6,
    7,
    29,
    26,
    8,
    9,
    10,
    11,
    30,
    27,
    12,
    13,
    14,
    15,
    31,
    34,
    20,
    21,
    22,
    23,
    35,
    36,
    37,
    38,
    39
]

for i in range(40):
    o = old.crop([i * 4, 0, i * 4 + 4, 4])
    img.paste(o, [table[i] * 4, 0])

img.save("dirt.png", "PNG")