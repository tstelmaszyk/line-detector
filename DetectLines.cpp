#include "DetectLines.h"
#include "LanePolynomial.h"
#include "LaneGeometry.h"

#include <opencv2/imgproc.hpp>

DetectLines::DetectLines(const VideoCaracteristics& video, const LaneConfig& config, ImageSink& debug_sink)
    : video_properties(video), config(config), debug_sink(debug_sink),
      mask(video, config, debug_sink),
      perspective(video, config, debug_sink),
      search(video, config, debug_sink),
      overlay(perspective, debug_sink)
{
}

LaneModel DetectLines::draw_lines(const cv::Mat& frame_to_compute, cv::Mat& frame_with_lines)
{
    cv::Mat binary;
    mask.compute(frame_to_compute, binary);

    cv::Mat bev;
    perspective.toBev(binary, bev);
    debug_sink.save("debug_02_bev.jpg", bev);

    const LanePixels px = search.search(bev);

    LaneModel model;
    model.left  = LanePolynomial::fit(px.left,  config.windowMinPix);
    model.right = LanePolynomial::fit(px.right, config.windowMinPix);

    model = LaneGeometry::compute(model, video_properties, config);

    // debug_04_fit : polynomes gauche/droite traces sur une copie BGR de la BEV.
    if (model.left.valid && model.right.valid) {
        cv::Mat fit_dbg;
        cv::cvtColor(bev, fit_dbg, cv::COLOR_GRAY2BGR);
        std::vector<cv::Point> ptL, ptR;
        ptL.reserve(bev.rows); ptR.reserve(bev.rows);
        for (int y = 0; y < bev.rows; ++y) {
            ptL.emplace_back(cvRound(model.left.evalAt(y)),  y);
            ptR.emplace_back(cvRound(model.right.evalAt(y)), y);
        }
        cv::polylines(fit_dbg, ptL, false, cv::Scalar(0,   0, 255), 2);
        cv::polylines(fit_dbg, ptR, false, cv::Scalar(255, 0,   0), 2);
        debug_sink.save("debug_04_fit.jpg", fit_dbg);
    }

    overlay.render(frame_to_compute, model, frame_with_lines);
    return model;
}
