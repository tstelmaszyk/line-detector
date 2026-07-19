#include "PerspectiveView.h"
#include "SmartAssert.h"

#include <cmath>
#include <opencv2/imgproc.hpp>

PerspectiveView::PerspectiveView(const VideoCaracteristics& video, const LaneConfig& config, ImageSink& debug_sink)
    : video_properties(video), config(config), debug_sink(debug_sink),
      bev_size(video.image_size)
{
    const float W = static_cast<float>(video_properties.width_pixel);
    const float H = static_cast<float>(video_properties.height_pixel);

    const float topY    = config.src_top_y_ratio * H;
    const float topHalf = config.src_top_width_ratio * W;

    // Quad source : trapeze (haut retreci vers l'horizon, bas plein cadre).
    src_quad = {
        cv::Point2f(W / 2.0f - topHalf, topY), // haut gauche
        cv::Point2f(W / 2.0f + topHalf, topY), // haut droit
        cv::Point2f(W,                  H),    // bas droit
        cv::Point2f(0.0f,               H)     // bas gauche
    };

    // Rectangle BEV, avec marge laterale pour laisser respirer les virages.
    const float margin = config.bev_margin_ratio * W;
    const std::vector<cv::Point2f> dst_quad = {
        cv::Point2f(margin,     0.0f),
        cv::Point2f(W - margin, 0.0f),
        cv::Point2f(W - margin, H),
        cv::Point2f(margin,     H)
    };

    M    = cv::getPerspectiveTransform(src_quad, dst_quad);
    Minv = cv::getPerspectiveTransform(dst_quad, src_quad);
    SMART_ASSERT(std::abs(cv::determinant(M)) > 1e-6,
                 "PerspectiveView: homographie degeneree (points colineaires ?)");
}

void PerspectiveView::toBev(const cv::Mat& src, cv::Mat& bev) const
{
    SMART_ASSERT(!src.empty(), "toBev: entree vide");
    SMART_ASSERT(src.size() == video_properties.image_size, "toBev: taille != VideoCaracteristics");
    cv::warpPerspective(src, bev, M, bev_size, cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0));
}

void PerspectiveView::warpBack(const cv::Mat& bev, cv::Mat& dst) const
{
    SMART_ASSERT(!bev.empty(), "warpBack: entree vide");
    cv::warpPerspective(bev, dst, Minv, video_properties.image_size,
                        cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0));
}

cv::Size PerspectiveView::bevSize() const { return bev_size; }
const std::vector<cv::Point2f>& PerspectiveView::sourceQuad() const { return src_quad; }
