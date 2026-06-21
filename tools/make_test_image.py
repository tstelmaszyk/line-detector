#!/usr/bin/env python3
"""Genere une image de test (img_piste/img2.jpg) representant une route
avec deux lignes de voie, pour faire tourner le pipeline de detection
sans avoir besoin d'une vraie photo ou d'une camera."""

from PIL import Image, ImageDraw
import os

W, H = 1280, 720

img = Image.new("RGB", (W, H), (110, 110, 110))  # route grise
draw = ImageDraw.Draw(img)

# Ciel plus clair dans la moitie haute (juste pour le realisme)
draw.rectangle([0, 0, W, H // 2], fill=(170, 190, 210))

# Deux lignes de voie blanches en perspective (quasi verticales -> gardees
# par le filtre d'angle > 40 deg du pipeline).
draw.line([(220, H), (560, H // 2)], fill=(255, 255, 255), width=12)
draw.line([(1060, H), (720, H // 2)], fill=(255, 255, 255), width=12)

os.makedirs("img_piste", exist_ok=True)
out = "img_piste/img2.jpg"
img.save(out, quality=90)
print(f"Image de test creee : {out} ({W}x{H})")
