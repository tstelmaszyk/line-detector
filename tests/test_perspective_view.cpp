#include "doctest.h"
#include "PerspectiveView.h"
#include "LaneConfig.h"
#include "VideoCaracteristics.h"
#include "NullImageSink.h"
#include <opencv2/core.hpp>

TEST_CASE("toBev produit une image a la taille BEV") {
    cv::Mat img(720, 1280, CV_8UC1, cv::Scalar(0));
    VideoCaracteristics video(img);
    LaneConfig config;
    NullImageSink sink;
    PerspectiveView pv(video, config, sink);

    cv::Mat bev;
    pv.toBev(img, bev);
    CHECK(bev.size() == pv.bevSize());
}

TEST_CASE("warpBack apres toBev revient a la taille d'origine") {
    cv::Mat img(720, 1280, CV_8UC1, cv::Scalar(0));
    VideoCaracteristics video(img);
    LaneConfig config;
    NullImageSink sink;
    PerspectiveView pv(video, config, sink);

    cv::Mat bev, back;
    pv.toBev(img, bev);
    pv.warpBack(bev, back);
    CHECK(back.size() == img.size());
}

TEST_CASE("le quad source a 4 sommets dans l'image") {
    cv::Mat img(720, 1280, CV_8UC1, cv::Scalar(0));
    VideoCaracteristics video(img);
    LaneConfig config;
    NullImageSink sink;
    PerspectiveView pv(video, config, sink);

    const auto& quad = pv.sourceQuad();
    REQUIRE(quad.size() == 4);
    for (const auto& p : quad) {
        CHECK(p.x >= 0.0f); CHECK(p.x <= 1280.0f);
        CHECK(p.y >= 0.0f); CHECK(p.y <= 720.0f);
    }
}
