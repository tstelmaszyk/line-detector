#pragma once

#include <vector>
#include <opencv2/core.hpp>

/*!
*  \brief Polynome de voie d'ordre 2, parametre par y : x = a*y^2 + b*y + c.
*
*  On parametre par y (et non x) car les voies sont ~verticales dans l'image,
*  ce qui evite les pentes infinies. `valid` distingue un fit reussi d'un cote
*  absent (aléa de la route, pas une erreur -> drapeau, pas d'assert).
*/
struct LanePolynomial {
    double a = 0.0, b = 0.0, c = 0.0;
    bool   valid = false;

    double evalAt(double y) const;

    // Ajuste au sens des moindres carres. Renvoie valid=false si
    // points.size() < minPoints (cote absent / trop peu de pixels).
    static LanePolynomial fit(const std::vector<cv::Point>& points, int minPoints = 3);
};
