#ifndef _h_video_carateristics_
#define _h_video_carateristics_

#include "projectTypes.h"
#include <opencv2/opencv.hpp>

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

#endif