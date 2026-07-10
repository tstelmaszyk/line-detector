#!/usr/bin/env python3
"""Genere des images de test dans img_piste/ pour la detection de voie :
- img2.jpg      : voie droite symetrique (compat historique)
- straight.jpg  : voie droite symetrique
- curved.jpg    : voie courbe (virage a droite)
- shifted.jpg   : voie decalee (vehicule off-center)
- dashed.jpg    : ligne droite pointillee d'un cote (teste la reconstruction)
"""

from PIL import Image, ImageDraw
import os

W, H = 1280, 720


def base_road():
    img = Image.new("RGB", (W, H), (110, 110, 110))
    draw = ImageDraw.Draw(img)
    draw.rectangle([0, 0, W, H // 2], fill=(170, 190, 210))  # ciel
    return img, draw


def straight(left_x=440, right_x=840):
    img, draw = base_road()
    draw.line([(left_x,  H), (left_x,  H // 2)], fill=(255, 255, 255), width=14)
    draw.line([(right_x, H), (right_x, H // 2)], fill=(255, 255, 255), width=14)
    return img


def curved():
    img, draw = base_road()
    # Deux polylignes qui derivent vers la droite en montant (virage a droite).
    left, right = [], []
    for i in range(11):
        t = i / 10.0
        y = H - int(t * (H // 2))
        dx = int(120 * t * t)  # courbure
        left.append((440 + dx, y))
        right.append((840 + dx, y))
    draw.line(left,  fill=(255, 255, 255), width=14, joint="curve")
    draw.line(right, fill=(255, 255, 255), width=14, joint="curve")
    return img


def dashed():
    img, draw = base_road()
    draw.line([(440, H), (440, H // 2)], fill=(255, 255, 255), width=14)  # gauche pleine
    # droite pointillee
    for i in range(0, 6):
        y0 = H - int((i / 6.0) * (H // 2))
        y1 = H - int(((i + 0.5) / 6.0) * (H // 2))
        draw.line([(840, y0), (840, y1)], fill=(255, 255, 255), width=14)
    return img


def main():
    os.makedirs("img_piste", exist_ok=True)
    images = {
        "img_piste/img2.jpg":     straight(),
        "img_piste/straight.jpg": straight(),
        "img_piste/curved.jpg":   curved(),
        "img_piste/shifted.jpg":  straight(540, 940),
        "img_piste/dashed.jpg":   dashed(),
    }
    for path, img in images.items():
        img.save(path, quality=90)
        print(f"Image de test creee : {path} ({W}x{H})")


if __name__ == "__main__":
    main()
