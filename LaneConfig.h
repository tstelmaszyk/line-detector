#pragma once

#include <opencv2/core.hpp>

/*!
*  \brief Configuration centralisee de la chaine de detection de voie.
*
*  Regroupe tous les reglages (seuillage, calibration BEV, fenetres glissantes).
*  Construit dans main.cpp avec des valeurs par defaut, injecte par valeur const
*  dans chaque composant. Les points source de la BEV se derivent des ratios
*  ci-dessous (cf. spec 5.1) et sont le seul reglage lie au montage camera.
*/
struct LaneConfig {
    // --- Seuillage (LaneMask) ---
    int       whiteThreshold  = 200;        // luminance mini du marquage blanc
    cv::Vec2i yellowHue        = {15, 35};  // plage de teinte jaune (HSV)
    int       sobelKernel      = 3;
    int       sobelThreshLow   = 30;
    int       sobelThreshHigh  = 150;

    // --- BEV (PerspectiveView) ---
    float srcTopWidthRatio = 0.10f;  // demi-largeur du bord haut du quad (fraction de W)
    float srcTopYRatio     = 0.62f;  // hauteur du bord haut (fraction de H)
    float bevMarginRatio   = 0.15f;  // marge laterale du rectangle BEV (fraction de W)

    // --- Fenetres glissantes (SlidingWindowSearch) ---
    int windowCount  = 9;
    int windowMargin = 60;
    int windowMinPix = 50;

    // --- Reconstruction / sanite (LaneGeometry) ---
    double defaultLaneWidthPx = 0.0; // 0 = pas de reconstruction du cote manquant
};
