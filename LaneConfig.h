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
    int       blurKernel       = 5;         // flou avant Sobel : tue la texture asphalte (impair)
    int       sobelKernel      = 3;
    int       sobelThreshLow   = 80;
    int       sobelThreshHigh  = 150;
    float     sobelNearCutoffRatio = 0.6f;  // Sobel ignore sous cette fraction de H : dans le champ proche (bas) le grain domine et survit au flou, le blanc/jaune y suffisent
    int       morphKernel      = 3;         // ouverture morpho du masque final : efface les mouchetures (impair)

    // --- BEV (PerspectiveView) ---
    // Cales sur img_piste/img1.png (vraie image camera) : le bord haut du trapeze
    // doit remonter vers le point de fuite (~45% de H) pour une vraie vue de dessus.
    // Regler en inspectant out/debug_02a_trapeze.jpg et out/debug_02b_bev_color.jpg.
    float srcTopWidthRatio = 0.18f;  // demi-largeur du bord haut du quad (fraction de W)
    float srcTopYRatio     = 0.45f;  // hauteur du bord haut (fraction de H)
    float bevMarginRatio   = 0.20f;  // marge laterale du rectangle BEV (fraction de W)

    // --- Fenetres glissantes (SlidingWindowSearch) ---
    int   windowCount       = 9;
    int   windowMargin      = 60;
    int   firstWindowMargin = 100;          // marge elargie de la 1re fenetre (bas) : le champ proche balaie vite en courbe et il n'y a pas d'a priori, une marge normale rate la queue au ras du bas
    int   windowMinPix      = 50;
    float histogramBandRatio = 0.25f;       // fraction basse de H pour l'histogramme de base : petit = ancre la base sur le tout-bas des lignes (les fortes courbures y sont decalees de la moyenne moitie-basse)

    // --- Reconstruction / sanite (LaneGeometry) ---
    double defaultLaneWidthPx = 0.0; // 0 = pas de reconstruction du cote manquant
};
