#pragma once

#include "projectTypes.h"
#include <opencv2/core.hpp>

using namespace cv;

struct VideoCaracteristics{
    cv::Size image_size;
    DimensionImage width_pixel ;
    DimensionImage height_pixel ;
    
    VideoCaracteristics(const Mat& reference_frame):    image_size(reference_frame.size()),
                                                        width_pixel(reference_frame.size().width),
                                                        height_pixel(reference_frame.size().height)

        {
        }

};