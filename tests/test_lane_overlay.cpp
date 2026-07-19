#include "doctest.h"
#include "LaneOverlay.h"
#include "PerspectiveView.h"
#include "LaneModel.h"
#include "LaneConfig.h"
#include "VideoCaracteristics.h"
#include "NullImageSink.h"
#include <opencv2/core.hpp>

TEST_CASE("render preserve taille/type et modifie des pixels quand la voie est detectee") {
    cv::Mat img(720, 1280, CV_8UC3, cv::Scalar(100, 100, 100));
    VideoCaracteristics video(img);
    LaneConfig config;
    NullImageSink sink;
    PerspectiveView pv(video, config, sink);
    LaneOverlay overlay(pv, sink);

    LaneModel m;
    m.left.quadratic_coefficient = 0;  m.left.linear_coefficient = 0;  m.left.constant_coefficient = 400;  m.left.valid = true;
    m.right.quadratic_coefficient = 0; m.right.linear_coefficient = 0; m.right.constant_coefficient = 800; m.right.valid = true;
    m.lane_detected = true;

    cv::Mat out;
    overlay.render(img, m, out);

    CHECK(out.size() == img.size());
    CHECK(out.type() == img.type());
    CHECK(cv::countNonZero(out.reshape(1) != img.reshape(1)) > 0);
}

TEST_CASE("render sans voie detectee renvoie une copie inchangee") {
    cv::Mat img(720, 1280, CV_8UC3, cv::Scalar(100, 100, 100));
    VideoCaracteristics video(img);
    LaneConfig config;
    NullImageSink sink;
    PerspectiveView pv(video, config, sink);
    LaneOverlay overlay(pv, sink);

    LaneModel m; // laneDetected = false
    cv::Mat out;
    overlay.render(img, m, out);
    CHECK(cv::countNonZero(out.reshape(1) != img.reshape(1)) == 0);
}
