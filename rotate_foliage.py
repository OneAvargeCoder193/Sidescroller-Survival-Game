from PIL import Image, ImageDraw

old = Image.open("old_gf.png")

img = Image.new("RGBA", (96, 4))

for i in range(24):
    c = old.crop([i * 4, 0, i * 4 + 4, 4])
    cr = c.rotate(i // 6 * -90)
    img.paste(cr, [i * 4, 0])

img.save("grass_foliage.png", "PNG")