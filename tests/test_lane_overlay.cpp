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
    m.left.a = 0;  m.left.b = 0;  m.left.c = 400;  m.left.valid = true;
    m.right.a = 0; m.right.b = 0; m.right.c = 800; m.right.valid = true;
    m.laneDetected = true;

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
