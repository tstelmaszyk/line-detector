#include "doctest.h"
#include "DetectLines.h"
#include "LaneConfig.h"
#include "VideoCaracteristics.h"
#include "NullImageSink.h"
#include <opencv2/imgproc.hpp>

// Construit une image BGR de route grise avec deux bandes blanches verticales.
static cv::Mat makeLaneImage(int W, int H, int leftX, int rightX) {
    cv::Mat img(H, W, CV_8UC3, cv::Scalar(110, 110, 110));
    cv::line(img, {leftX,  H - 1}, {leftX,  H / 2}, cv::Scalar(255, 255, 255), 14);
    cv::line(img, {rightX, H - 1}, {rightX, H / 2}, cv::Scalar(255, 255, 255), 14);
    return img;
}

TEST_CASE("draw_lines renvoie une image a la taille d'origine") {
    cv::Mat img = makeLaneImage(1280, 720, 440, 840);
    VideoCaracteristics video(img);
    LaneConfig config;
    config.defaultLaneWidthPx = 640.0;
    NullImageSink sink;
    DetectLines det(video, config, sink);

    cv::Mat out;
    const LaneModel m = det.draw_lines(img, out);
    CHECK(out.size() == img.size());
    CHECK(out.type() == img.type());
}
