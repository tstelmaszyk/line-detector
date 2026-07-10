#pragma once

#include "LanePolynomial.h"

/*!
*  \brief Resultat complet de la detection : modele de voie + signal de pilotage.
*
*  Les polynomes sont exprimes en pixels BEV. Le signal (offset, courbure) est
*  rempli par LaneGeometry. Consomme plus tard par le module de controle.
*/
struct LaneModel {
    LanePolynomial left;
    LanePolynomial right;
    bool   laneDetected = false;

    double lateralOffsetPx   = 0.0; // ecart vehicule <-> centre voie, au bas de l'image
    double normalizedOffset  = 0.0; // offset / demi-largeur voie ; <0 = trop a gauche
    double curvatureRadiusPx = 0.0; // rayon de courbure (grand = quasi droit)
};
