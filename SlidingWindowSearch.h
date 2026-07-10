#pragma once

#include <vector>
#include <opencv2/core.hpp>
#include "VideoCaracteristics.h"
#include "LaneConfig.h"
#include "ImageSink.h"

/*!
*  \brief Pixels de voie separes gauche/droite (coordonnees en pixels BEV).
*/
struct LanePixels {
    std::vector<cv::Point> left;
    std::vector<cv::Point> right;
};

/*!
*  \brief Histogramme (moitie basse) pour trouver les deux bases, puis fenetres
*  glissantes de bas en haut pour collecter les pixels de chaque cote. Une
*  fenetre sans assez de pixels NE bouge PAS (garde la position precedente).
*/
class SlidingWindowSearch {
    public:
        SlidingWindowSearch(const VideoCaracteristics& video, const LaneConfig& config, ImageSink& debug_sink);
        LanePixels search(const cv::Mat& bev_binary) const;

    private:
        const VideoCaracteristics video_properties;
        const LaneConfig config;
        ImageSink& debug_sink;
};
