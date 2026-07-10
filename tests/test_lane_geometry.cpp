#include "doctest.h"
#include "LaneGeometry.h"
#include "LaneModel.h"
#include "LaneConfig.h"
#include "VideoCaracteristics.h"
#include <opencv2/core.hpp>

static LanePolynomial straight(double x) {
    LanePolynomial p; p.a = 0; p.b = 0; p.c = x; p.valid = true; return p;
}

TEST_CASE("voie symetrique -> offset nul") {
    cv::Mat ref(720, 1280, CV_8UC3);
    VideoCaracteristics video(ref);
    LaneConfig config;
    LaneModel m;
    m.left  = straight(440);  // centre image = 640, centre voie = (440+840)/2 = 640
    m.right = straight(840);
    const LaneModel r = LaneGeometry::compute(m, video, config);
    REQUIRE(r.laneDetected);
    CHECK(r.lateralOffsetPx == doctest::Approx(0.0));
    CHECK(r.normalizedOffset == doctest::Approx(0.0));
}

TEST_CASE("centre de voie a droite du centre image -> offset negatif") {
    cv::Mat ref(720, 1280, CV_8UC3);
    VideoCaracteristics video(ref);
    LaneConfig config;
    LaneModel m;
    m.left  = straight(500);
    m.right = straight(900);  // centre voie = 700 > 640 -> vehicule a gauche
    const LaneModel r = LaneGeometry::compute(m, video, config);
    REQUIRE(r.laneDetected);
    CHECK(r.lateralOffsetPx < 0.0);
    CHECK(r.normalizedOffset < 0.0);
}

TEST_CASE("un seul cote valide + defaultLaneWidthPx -> reconstruction") {
    cv::Mat ref(720, 1280, CV_8UC3);
    VideoCaracteristics video(ref);
    LaneConfig config;
    config.defaultLaneWidthPx = 400.0;
    LaneModel m;
    m.left = straight(440); // right invalide
    const LaneModel r = LaneGeometry::compute(m, video, config);
    REQUIRE(r.laneDetected);
    CHECK(r.right.valid);
    CHECK(r.right.c == doctest::Approx(840.0)); // 440 + 400
}

TEST_CASE("aucun cote valide -> laneDetected faux") {
    cv::Mat ref(720, 1280, CV_8UC3);
    VideoCaracteristics video(ref);
    LaneConfig config;
    LaneModel m;
    const LaneModel r = LaneGeometry::compute(m, video, config);
    CHECK_FALSE(r.laneDetected);
}
