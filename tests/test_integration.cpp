#include "doctest.h"
#include "DetectLines.h"
#include "LaneConfig.h"
#include "VideoCaracteristics.h"
#include "NullImageSink.h"
#include "SlidingWindowSearch.h"
#include "LanePolynomial.h"
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
    config.default_lane_width_px = W * 0.5;
    NullImageSink sink;
    DetectLines det(video, config, sink);

    cv::Mat out;
    const LaneModel m = det.draw_lines(img, out);
    REQUIRE(m.lane_detected);
    CHECK(std::abs(m.normalized_offset) < 0.15);
}

// Voie decalee a droite -> centre de voie a droite du centre image -> offset < 0.
TEST_CASE("pipeline complet : voie decalee a droite -> offset negatif") {
    const int W = 1280, H = 720;
    cv::Mat img = makeLaneImage(W, H, 540, 940); // decalee +100
    VideoCaracteristics video(img);
    LaneConfig config;
    config.default_lane_width_px = W * 0.5;
    NullImageSink sink;
    DetectLines det(video, config, sink);

    cv::Mat out;
    const LaneModel m = det.draw_lines(img, out);
    REQUIRE(m.lane_detected);
    CHECK(m.normalized_offset < 0.0);
}

// Voie courbe (BEV binaire) : verifie que SlidingWindowSearch + fit suivent
// la courbure sans l'aplatir.
// Courbe : x = base + K*(H-1-y)^2, courbure positive (stripes courbees vers la droite
// en montant). K=0.0003 donne ~155 px de decalage au sommet -> terme quadratique
// bien au-dessus du bruit.
TEST_CASE("recherche + fit sur voie courbe -> terme quadratique non nul") {
    const int W = 1280, H = 720;
    const double K       = 3e-4;  // courbure ; a attendu > 0
    const int leftBase   = 300;   // dans [0, W/2)
    const int rightBase  = 900;   // dans [W/2, W)
    const int stripeHalf = 8;     // demi-largeur de la stripe en pixels

    // Image BEV binaire : fond noir, deux stripes blanches courbes.
    cv::Mat bev(H, W, CV_8UC1, cv::Scalar(0));
    for (int y = 0; y < H; ++y) {
        const double yFromBottom = static_cast<double>(H - 1 - y);
        const int xL = cvRound(leftBase  + K * yFromBottom * yFromBottom);
        const int xR = cvRound(rightBase + K * yFromBottom * yFromBottom);
        for (int dx = -stripeHalf; dx <= stripeHalf; ++dx) {
            if (xL + dx >= 0 && xL + dx < W) bev.at<uchar>(y, xL + dx) = 255;
            if (xR + dx >= 0 && xR + dx < W) bev.at<uchar>(y, xR + dx) = 255;
        }
    }

    cv::Mat ref(H, W, CV_8UC3);
    VideoCaracteristics video(ref);
    LaneConfig config;
    config.window_count  = 9;
    config.window_margin = 80;
    config.window_min_pix = 5;
    NullImageSink sink;
    SlidingWindowSearch searcher(video, config, sink);

    const LanePixels px = searcher.search(bev);
    const LanePolynomial fitL = LanePolynomial::fit(px.left,  config.window_min_pix);
    const LanePolynomial fitR = LanePolynomial::fit(px.right, config.window_min_pix);

    REQUIRE(fitL.valid);
    REQUIRE(fitR.valid);
    // Le terme quadratique doit refleter la courbure positive dessinee.
    CHECK(fitL.quadratic_coefficient > 1e-4);
    CHECK(fitR.quadratic_coefficient > 1e-4);
}
