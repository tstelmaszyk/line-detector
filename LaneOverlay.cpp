#include "LaneOverlay.h"
#include "SmartAssert.h"

#include <cstdio>
#include <vector>
#include <opencv2/imgproc.hpp>

LaneOverlay::LaneOverlay(const PerspectiveView& perspective, ImageSink& debug_sink)
    : perspective(perspective), debug_sink(debug_sink)
{
}

void LaneOverlay::render(const cv::Mat& original_bgr, const LaneModel& model, cv::Mat& output) const
{
    SMART_ASSERT(!original_bgr.empty(), "render: image d'origine vide");
    SMART_ASSERT(original_bgr.channels() == 3, "render: attend une image BGR");

    output = original_bgr.clone();
    if (!model.laneDetected) {
        debug_sink.save("debug_05_overlay.jpg", output);
        return;
    }

    const cv::Size bev = perspective.bevSize();
    cv::Mat lane_bev(bev, CV_8UC3, cv::Scalar(0, 0, 0));

    // Polygone entre les deux polynomes, echantillonne le long de y.
    std::vector<cv::Point> poly;
    for (int y = 0; y < bev.height; ++y)
        poly.emplace_back(cvRound(model.left.eval_at(y)), y);
    for (int y = bev.height - 1; y >= 0; --y)
        poly.emplace_back(cvRound(model.right.eval_at(y)), y);
    const std::vector<std::vector<cv::Point>> polys = { poly };
    cv::fillPoly(lane_bev, polys, cv::Scalar(0, 255, 0));

    // Retour en perspective image et fusion.
    cv::Mat lane_img;
    perspective.warpBack(lane_bev, lane_img);
    // src1==dst est intentionnel et supporte par addWeighted (in-place aliasing OK).
    cv::addWeighted(output, 1.0, lane_img, 0.3, 0.0, output);

    // HUD : offset normalise et rayon de courbure.
    char hud[128];
    std::snprintf(hud, sizeof(hud), "offset=%.2f  R=%.0fpx",
                  model.normalizedOffset, model.curvatureRadiusPx);
    cv::putText(output, hud, cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 1.0,
                cv::Scalar(0, 0, 255), 2);

    debug_sink.save("debug_05_overlay.jpg", output);
}
