#include "doctest.h"
#include "SlidingWindowSearch.h"
#include "LaneConfig.h"
#include "VideoCaracteristics.h"
#include "NullImageSink.h"
#include <opencv2/imgproc.hpp>

TEST_CASE("search separe deux bandes verticales en cotes gauche/droite") {
    cv::Mat bev(720, 1280, CV_8UC1, cv::Scalar(0));
    cv::rectangle(bev, cv::Rect(300, 0, 12, 720), cv::Scalar(255), cv::FILLED); // gauche
    cv::rectangle(bev, cv::Rect(900, 0, 12, 720), cv::Scalar(255), cv::FILLED); // droite

    VideoCaracteristics video(bev);
    LaneConfig config;
    NullImageSink sink;
    SlidingWindowSearch sws(video, config, sink);

    const LanePixels px = sws.search(bev);
    REQUIRE(px.left.size()  > 100);
    REQUIRE(px.right.size() > 100);

    double lx = 0; for (const auto& p : px.left)  lx += p.x; lx /= px.left.size();
    double rx = 0; for (const auto& p : px.right) rx += p.x; rx /= px.right.size();
    CHECK(lx == doctest::Approx(306).epsilon(0.1));
    CHECK(rx == doctest::Approx(906).epsilon(0.1));
}
