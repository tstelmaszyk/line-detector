#pragma once

#include <opencv2/core.hpp>
#include "VideoCaracteristics.h"
#include "LaneConfig.h"
#include "ImageSink.h"

/*!
*  \brief Produit un binaire des marquages : blanc (luminance) + jaune (HSV) +
*  bords (Sobel x). Remplace le trio grayscale/blur/Canny de l'ancien pipeline.
*  Entree : BGR (telle que cv::imread). Sortie : CV_8UC1 a valeurs {0,255}.
*/
class LaneMask {
    public:
        LaneMask(const VideoCaracteristics& video, const LaneConfig& config, ImageSink& debug_sink);
        void compute(const cv::Mat& bgr, cv::Mat& binary);

    private:
        const VideoCaracteristics video_properties;
        const LaneConfig config;
        ImageSink& debug_sink;
};
