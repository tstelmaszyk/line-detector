#pragma once

#include <vector>
#include <opencv2/core.hpp>
#include "VideoCaracteristics.h"
#include "LaneConfig.h"
#include "ImageSink.h"

/*!
*  \brief Vue de dessus (bird's eye view). Construit les homographies directe et
*  inverse a partir d'un quadrilatere source (derive des dimensions, comme
*  l'ancien trapeze de RegionOfInterest) vers un rectangle BEV plein cadre.
*/
class PerspectiveView {
    public:
        PerspectiveView(const VideoCaracteristics& video, const LaneConfig& config, ImageSink& debug_sink);

        void toBev(const cv::Mat& src, cv::Mat& bev) const;     // image -> BEV
        void warpBack(const cv::Mat& bev, cv::Mat& dst) const;  // BEV -> image
        cv::Size bevSize() const;
        const std::vector<cv::Point2f>& sourceQuad() const;

    private:
        const VideoCaracteristics video_properties;
        const LaneConfig config;
        ImageSink& debug_sink;
        std::vector<cv::Point2f> src_quad;
        cv::Size bev_size;
        cv::Mat M;     // src -> BEV
        cv::Mat Minv;  // BEV -> src
};
