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

    // Debug calibration BEV (inspectables a l'oeil, contrairement au masque warpe) :
    // le trapeze source dessine sur l'image couleur, et le warp couleur en vue de dessus.
    {
        cv::Mat trapeze = frame_to_compute.clone();
        const std::vector<cv::Point2f>& quad = perspective.sourceQuad();
        std::vector<cv::Point> poly;
        poly.reserve(quad.size());
        for (const cv::Point2f& p : quad) poly.emplace_back(cvRound(p.x), cvRound(p.y));
        cv::polylines(trapeze, poly, /*isClosed=*/true, cv::Scalar(0, 0, 255), 3);
        debug_sink.save("debug_02a_trapeze.jpg", trapeze);

        cv::Mat bev_color;
        perspective.toBev(frame_to_compute, bev_color);
        debug_sink.save("debug_02b_bev_color.jpg", bev_color);
    }

    const LanePixels px = search.search(bev);

    LaneModel model;
    model.left  = LanePolynomial::fit(px.left,  config.window_min_pix);
    model.right = LanePolynomial::fit(px.right, config.window_min_pix);

    model = LaneGeometry::compute(model, video_properties, config);

    // debug_04_fit : polynomes gauche/droite traces sur une copie BGR de la BEV.
    if (model.left.valid && model.right.valid) {
        cv::Mat fit_dbg;
        cv::cvtColor(bev, fit_dbg, cv::COLOR_GRAY2BGR);
        std::vector<cv::Point> ptL, ptR;
        ptL.reserve(bev.rows); ptR.reserve(bev.rows);
        for (int y = 0; y < bev.rows; ++y) {
            ptL.emplace_back(cvRound(model.left.eval_at(y)),  y);
            ptR.emplace_back(cvRound(model.right.eval_at(y)), y);
        }
        cv::polylines(fit_dbg, ptL, false, cv::Scalar(0,   0, 255), 2);
        cv::polylines(fit_dbg, ptR, false, cv::Scalar(255, 0,   0), 2);
        debug_sink.save("debug_04_fit.jpg", fit_dbg);
    }

    overlay.render(frame_to_compute, model, frame_with_lines);
    return model;
}
