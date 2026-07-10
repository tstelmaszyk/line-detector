#pragma once

#include <opencv2/core.hpp>
#include "VideoCaracteristics.h"
#include "LaneConfig.h"
#include "ImageSink.h"
#include "LaneModel.h"
#include "LaneMask.h"
#include "PerspectiveView.h"
#include "SlidingWindowSearch.h"
#include "LaneOverlay.h"

/*!
*  \brief Orchestre la chaine de detection de voie : masque -> BEV -> fenetres
*  glissantes -> fit polynomial -> geometrie -> overlay. Renvoie le LaneModel
*  (signal de pilotage) et dessine le resultat dans frame_with_lines.
*/
class DetectLines {
    public:
        DetectLines(const VideoCaracteristics& video, const LaneConfig& config, ImageSink& debug_sink);
        LaneModel draw_lines(const cv::Mat& frame_to_compute, cv::Mat& frame_with_lines);

    private:
        const VideoCaracteristics video_properties;
        const LaneConfig config;
        ImageSink& debug_sink;
        LaneMask mask;
        PerspectiveView perspective;
        SlidingWindowSearch search;
        LaneOverlay overlay;
};
