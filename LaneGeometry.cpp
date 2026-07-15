#include "LaneGeometry.h"
#include "SmartAssert.h"

#include <cmath>

LaneModel LaneGeometry::compute(LaneModel model,
                                const VideoCaracteristics& video,
                                const LaneConfig& config)
{
    const double yMax = static_cast<double>(video.height_pixel) - 1.0;
    const double imageCenterX = static_cast<double>(video.width_pixel) / 2.0;

    // Reconstruction du cote manquant par decalage d'une largeur de voie.
    if (model.left.valid && !model.right.valid && config.defaultLaneWidthPx > 0.0) {
        model.right = model.left;
        model.right.c += config.defaultLaneWidthPx;
        model.reconstructed = true;
    } else if (model.right.valid && !model.left.valid && config.defaultLaneWidthPx > 0.0) {
        model.left = model.right;
        model.left.c -= config.defaultLaneWidthPx;
        model.reconstructed = true;
    }

    if (!model.left.valid || !model.right.valid) {
        model.laneDetected = false;
        model.lateralOffsetPx = 0.0;
        model.normalizedOffset = 0.0;
        model.curvatureRadiusPx = 0.0;
        return model;
    }

    const double xLeft  = model.left.evalAt(yMax);
    const double xRight = model.right.evalAt(yMax);

    // Sanite : largeur de voie positive plausible (aléa -> drapeau, pas assert).
    if (xRight - xLeft <= 1.0) {
        model.laneDetected = false;
        model.lateralOffsetPx = 0.0;
        model.normalizedOffset = 0.0;
        model.curvatureRadiusPx = 0.0;
        return model;
    }

    const double laneCenter = (xLeft + xRight) / 2.0;
    const double halfWidth  = (xRight - xLeft) / 2.0;

    model.lateralOffsetPx  = imageCenterX - laneCenter;
    model.normalizedOffset = model.lateralOffsetPx / halfWidth;

    // Rayon de courbure du cote gauche (les deux sont ~paralleles en BEV).
    const double a = model.left.a;
    const double b = model.left.b;
    const double denom = std::abs(2.0 * a);
    // |2a| < 1e-9 -> rayon > ~5e8 px, voie consideree droite
    if (denom < 1e-9) {
        model.curvatureRadiusPx = 1e12; // quasi-droit
    } else {
        const double slope = 2.0 * a * yMax + b;
        model.curvatureRadiusPx = std::pow(1.0 + slope * slope, 1.5) / denom;
    }

    SMART_ASSERT(std::isfinite(model.normalizedOffset), "normalizedOffset non fini");
    model.laneDetected = true;
    return model;
}
