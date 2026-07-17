#!/usr/bin/env python3
"""Aide a la calibration de la BEV (bird's eye view) SANS OpenCV.

Reproduit exactement l'homographie de PerspectiveView.cpp (trapeze source ->
rectangle BEV) a partir des memes ratios que LaneConfig.h, puis warpe l'image
COULEUR d'origine. Contrairement a out/debug_02_bev.jpg (qui warpe le masque
binaire bruite), on peut ainsi juger la geometrie a l'oeil : sur une image en
perspective bien calibree, les portions droites des lignes de voie doivent
ressortir verticales et paralleles.

Ne depend que de numpy + Pillow (pas d'OpenCV) : tourne directement sur l'hote.

Produit deux images :
  <out>/<nom>_trapeze.jpg   : trapeze source (rouge) trace sur l'image d'origine
  <out>/<nom>_bev.jpg       : image couleur warpee en vue de dessus

Exemple :
  python3 tools/bev_calibrate.py img_piste/img1.png \\
      --top-width 0.18 --top-y 0.45 --margin 0.20 --out out
"""

import argparse
import os

import numpy as np
from PIL import Image, ImageDraw


def find_coeffs(dst_quad, src_quad):
    """Coefficients PIL PERSPECTIVE mappant la sortie (BEV) vers la source.

    Equivalent de cv::getPerspectiveTransform(dst_quad, src_quad) : PIL applique
    la transformation inverse (pour chaque pixel de sortie, ou lire la source).
    """
    matrix = []
    for out_pt, in_pt in zip(dst_quad, src_quad):
        matrix.append([out_pt[0], out_pt[1], 1, 0, 0, 0,
                       -in_pt[0] * out_pt[0], -in_pt[0] * out_pt[1]])
        matrix.append([0, 0, 0, out_pt[0], out_pt[1], 1,
                       -in_pt[1] * out_pt[0], -in_pt[1] * out_pt[1]])
    A = np.array(matrix, dtype=float)
    B = np.array(src_quad, dtype=float).reshape(8)
    return np.linalg.solve(A, B)


def build_quads(W, H, top_width_ratio, top_y_ratio, margin_ratio):
    """Meme geometrie que PerspectiveView.cpp (ordre : HG, HD, BD, BG)."""
    top_y = top_y_ratio * H
    top_half = top_width_ratio * W
    src_quad = [
        (W / 2.0 - top_half, top_y),  # haut gauche
        (W / 2.0 + top_half, top_y),  # haut droit
        (W, H),                        # bas droit
        (0.0, H),                      # bas gauche
    ]
    margin = margin_ratio * W
    dst_quad = [
        (margin, 0.0),
        (W - margin, 0.0),
        (W - margin, H),
        (margin, H),
    ]
    return src_quad, dst_quad


def main():
    parser = argparse.ArgumentParser(description="Calibration visuelle de la BEV.")
    parser.add_argument("image", help="chemin de l'image source (perspective)")
    parser.add_argument("--top-width", type=float, default=0.18,
                        help="srcTopWidthRatio (demi-largeur du bord haut / W)")
    parser.add_argument("--top-y", type=float, default=0.45,
                        help="srcTopYRatio (hauteur du bord haut / H)")
    parser.add_argument("--margin", type=float, default=0.20,
                        help="bevMarginRatio (marge laterale du rectangle BEV / W)")
    parser.add_argument("--out", default="out", help="dossier de sortie")
    args = parser.parse_args()

    img = Image.open(args.image).convert("RGB")
    W, H = img.size
    src_quad, dst_quad = build_quads(W, H, args.top_width, args.top_y, args.margin)

    os.makedirs(args.out, exist_ok=True)
    name = os.path.splitext(os.path.basename(args.image))[0]

    # 1) trapeze source sur l'image couleur
    trap = img.copy()
    draw = ImageDraw.Draw(trap)
    draw.line([tuple(p) for p in src_quad] + [tuple(src_quad[0])],
              fill=(255, 0, 0), width=4)
    trap_path = os.path.join(args.out, f"{name}_trapeze.jpg")
    trap.save(trap_path)

    # 2) warp couleur en vue de dessus
    coeffs = find_coeffs(dst_quad, src_quad)
    bev = img.transform((W, H), Image.PERSPECTIVE, coeffs, resample=Image.BILINEAR)
    bev_path = os.path.join(args.out, f"{name}_bev.jpg")
    bev.save(bev_path)

    print(f"image {W}x{H}  top_width={args.top_width} top_y={args.top_y} "
          f"margin={args.margin}")
    print(f"src_quad = {[(round(x), round(y)) for x, y in src_quad]}")
    print(f"ecrit : {trap_path}")
    print(f"ecrit : {bev_path}")


if __name__ == "__main__":
    main()
