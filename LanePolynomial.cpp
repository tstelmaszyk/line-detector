#include "LanePolynomial.h"
#include "SmartAssert.h"

#include <cmath>

double LanePolynomial::evalAt(double y) const
{
    const double x = a*y*y + b*y + c;
    SMART_ASSERT(std::isfinite(x), "LanePolynomial::evalAt a produit une valeur non finie");
    return x;
}

LanePolynomial LanePolynomial::fit(const std::vector<cv::Point>& points, int minPoints)
{
    LanePolynomial poly;
    if (static_cast<int>(points.size()) < minPoints) {
        poly.valid = false;
        return poly;
    }

    const int n = static_cast<int>(points.size());
    cv::Mat A(n, 3, CV_64F);
    cv::Mat X(n, 1, CV_64F);
    for (int i = 0; i < n; ++i) {
        const double y = static_cast<double>(points[i].y);
        A.at<double>(i, 0) = y * y;
        A.at<double>(i, 1) = y;
        A.at<double>(i, 2) = 1.0;
        X.at<double>(i, 0) = static_cast<double>(points[i].x);
    }

    cv::Mat coeffs;
    const bool ok = cv::solve(A, X, coeffs, cv::DECOMP_SVD);
    SMART_ASSERT(ok, "cv::solve (SVD) a echoue sur un systeme non vide");

    poly.a = coeffs.at<double>(0, 0);
    poly.b = coeffs.at<double>(1, 0);
    poly.c = coeffs.at<double>(2, 0);
    poly.valid = true;
    return poly;
}
