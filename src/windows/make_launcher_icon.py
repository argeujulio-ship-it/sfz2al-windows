"""Create original Z2 launcher artwork. Requires Pillow; contains no game assets."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

root = Path(__file__).resolve().parent
image = Image.new("RGBA", (256, 256))
draw = ImageDraw.Draw(image)
draw.rounded_rectangle((8, 8, 248, 248), radius=48, fill="#101827", outline="#ffc543", width=10)
draw.polygon([(23, 155), (223, 45), (233, 91), (33, 201)], fill="#ba293c")
font = ImageFont.truetype("arialbd.ttf", 139)
draw.text((128, 126), "Z2", font=font, anchor="mm", fill="#ffce4b", stroke_width=4, stroke_fill="#101827")
draw.rounded_rectangle((68, 211, 188, 223), radius=6, fill="#39c9e2")
image.save(root / "launcher.ico", sizes=[(16,16), (20,20), (24,24), (32,32), (40,40), (48,48), (64,64), (128,128), (256,256)])
image.save(root / "launcher-icon-preview.png")
