#include "DetectLines.h"
#include "LanePolynomial.h"
#include "LaneGeometry.h"

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

    overlay.render(frame_to_compute, model, frame_with_lines);
    return model;
}
