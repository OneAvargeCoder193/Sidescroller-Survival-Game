from PIL import Image

img = Image.open("image.png")

out = Image.new("RGBA", img.size)
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

out.save("assets/textures/sun.png")
out_ring.save("assets/textures/sun_ring.png")