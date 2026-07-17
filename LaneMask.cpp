#include "LaneMask.h"
#include "SmartAssert.h"

#include <opencv2/imgproc.hpp>

LaneMask::LaneMask(const VideoCaracteristics& video, const LaneConfig& config, ImageSink& debug_sink)
    : video_properties(video), config(config), debug_sink(debug_sink)
{
}

void LaneMask::compute(const cv::Mat& bgr, cv::Mat& binary)
{
    SMART_ASSERT(!bgr.empty(), "LaneMask: image d'entree vide");
    SMART_ASSERT(bgr.channels() == 3, "LaneMask: attend une image BGR 3 canaux");
    SMART_ASSERT(bgr.size() == video_properties.image_size, "LaneMask: taille != VideoCaracteristics");

    // 1. Marquage blanc : forte luminance. BGR (imread charge en BGR), PAS RGB.
    cv::Mat gray;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    cv::Mat white;
    cv::threshold(gray, white, config.whiteThreshold, 255, cv::THRESH_BINARY);

    // 2. Marquage jaune : plage de teinte en HSV.
    cv::Mat hsv;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);
    cv::Mat yellow;
    cv::inRange(hsv,
                cv::Scalar(config.yellowHue[0], 80, 80),
                cv::Scalar(config.yellowHue[1], 255, 255),
                yellow);

    // 3. Gradient horizontal (Sobel x) : renforce les bords de marquage.
    //    Flou prealable pour ne pas reagir a la texture haute-frequence de l'asphalte.
    cv::Mat grayBlur;
    cv::GaussianBlur(gray, grayBlur, cv::Size(config.blurKernel, config.blurKernel), 0);
    cv::Mat sobelx, sobel_abs, sobel_bin;
    cv::Sobel(grayBlur, sobelx, CV_16S, 1, 0, config.sobelKernel);
    cv::convertScaleAbs(sobelx, sobel_abs);
    cv::inRange(sobel_abs, cv::Scalar(config.sobelThreshLow), cv::Scalar(config.sobelThreshHigh), sobel_bin);

    // 4. Combinaison OR des trois masques, puis ouverture morpho pour effacer les mouchetures isolees.
    binary = white | yellow | sobel_bin;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, {config.morphKernel, config.morphKernel});
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);

    SMART_ASSERT(binary.type() == CV_8UC1, "LaneMask: sortie non binaire mono-canal");
    debug_sink.save("debug_01_mask.jpg", binary);
}
