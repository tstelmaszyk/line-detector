#include "doctest.h"
#include "DetectLines.h"
#include "LaneConfig.h"
#include "VideoCaracteristics.h"
#include "NullImageSink.h"
#include <opencv2/imgproc.hpp>
#include <cmath>

static cv::Mat makeLaneImage(int W, int H, int leftX, int rightX) {
    cv::Mat img(H, W, CV_8UC3, cv::Scalar(110, 110, 110));
    cv::line(img, {leftX,  H - 1}, {leftX,  H / 2}, cv::Scalar(255, 255, 255), 14);
    cv::line(img, {rightX, H - 1}, {rightX, H / 2}, cv::Scalar(255, 255, 255), 14);
    return img;
}

// Entree miroir-symetrique autour de x=W/2 + transform symetrique -> offset ~ 0.
TEST_CASE("pipeline complet : voie symetrique -> offset proche de zero") {
    const int W = 1280, H = 720;
    cv::Mat img = makeLaneImage(W, H, 440, 840); // symetrique autour de 640
    VideoCaracteristics video(img);
    LaneConfig config;
    config.defaultLaneWidthPx = W * 0.5;
    NullImageSink sink;
    DetectLines det(video, config, sink);

    cv::Mat out;
    const LaneModel m = det.draw_lines(img, out);
    REQUIRE(m.laneDetected);
    CHECK(std::abs(m.normalizedOffset) < 0.15);
}

// Voie decalee a droite -> centre de voie a droite du centre image -> offset < 0.
TEST_CASE("pipeline complet : voie decalee a droite -> offset negatif") {
    const int W = 1280, H = 720;
    cv::Mat img = makeLaneImage(W, H, 540, 940); // decalee +100
    VideoCaracteristics video(img);
    LaneConfig config;
    config.defaultLaneWidthPx = W * 0.5;
    NullImageSink sink;
    DetectLines det(video, config, sink);

    cv::Mat out;
    const LaneModel m = det.draw_lines(img, out);
    REQUIRE(m.laneDetected);
    CHECK(m.normalizedOffset < 0.0);
}
