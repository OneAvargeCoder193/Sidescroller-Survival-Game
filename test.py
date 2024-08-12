from PIL import Image, ImageDraw

img = Image.open("image.png")

out = Image.new("RGBA", img.size)
draw = ImageDraw.Draw(out)

out_ring = Image.new("RGBA", img.size)

for y in range(img.height):
    for x in range(img.width):
        r = img.getpixel((x, y))[0]

        cx = x - img.width / 2
        cy = y - img.height / 2

        if cx * cx + cy * cy < 225:
            out.putpixel((x, y), (255, 255, 200, r))
        else:
            out_ring.putpixel((x, y), (255, 255, 200, r))

aac = Image.new("RGBA", (img.width * 2, img.height * 2))
draw = ImageDraw.Draw(aac)
draw.ellipse([aac.width / 2 - 30, aac.height / 2 - 30, aac.width / 2 + 31, aac.height / 2 + 31], outline=(255, 255, 200, 255), width=2)

out.alpha_composite(aac.resize(img.size))
out_ring.alpha_composite(aac.resize(img.size))

out.save("assets/textures/sun.png")
out_ring.save("assets/textures/sun_ring.png")