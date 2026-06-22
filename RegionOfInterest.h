#pragma once

#include <vector>
#include <opencv2/core.hpp>
#include "projectTypes.h"
#include "VideoCaracteristics.h"
#include "ImageSink.h"


using namespace cv;

/*!
*  \brief Mask a region of interest
*
*  In the above picture, there are some outliers; some edges from the other part of the road, from the landscape (mountains), etc. 
*  As our camera will be fixed, we can put a mask upon the image and keep only these lines that are interesting for our task. 
*  Thus, it will be very natural to draw a trapezium in order to keep only an area on where we should expect the road lines to be.  
*  \param 
*/
class RegionOfInterest
    {
        public:
            RegionOfInterest(const VideoCaracteristics& video_properties, ImageSink& debug_sink);

            /*!
            *  \brief Mask is applied to the frame sent
            */
            void apply_mask(cv::Mat &frame_to_mask);

        private: 
            const VideoCaracteristics video_properties ;
            ImageSink& debug_sink ;
            std::vector<Point> mask_vertex_pts;
            cv::Mat mask_to_apply  ;
            
            /*!
            *  \brief compute_point_coordinates
            *  (0,0) -- x
            *  |
            *  y
            *  Step 1 : we suppose top points are the same (middle of the picture)
            */
            void compute_trapeze_point_coordinates(std::vector<Point> &pts);
    };