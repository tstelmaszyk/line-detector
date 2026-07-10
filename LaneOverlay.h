#pragma once

#include <opencv2/core.hpp>
#include "LaneModel.h"
#include "PerspectiveView.h"
#include "ImageSink.h"

/*!
*  \brief Dessine la voie detectee (polygone entre les deux polynomes en BEV),
*  la ramene en perspective image (warp inverse), la fusionne sur l'image
*  d'origine et ajoute un HUD (offset + courbure).
*/
class LaneOverlay {
    public:
        LaneOverlay(const PerspectiveView& perspective, ImageSink& debug_sink);
        void render(const cv::Mat& original_bgr, const LaneModel& model, cv::Mat& output) const;

    private:
        const PerspectiveView& perspective;
        ImageSink& debug_sink;
};
