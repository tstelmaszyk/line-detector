#ifndef _detect_lines_h
#define _detect_lines_h

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <math.h>
#include "projectTypes.h"
#include "VideoCaracteristics.h"
#include "RegionOfInterest.h"


using namespace cv;

class DetectLines
    {
        public:
            DetectLines(const VideoCaracteristics& video_properties);
            void draw_lines (const Mat& frame_to_compute, Mat& frame_with_lines);

        private :
            const VideoCaracteristics video_properties ;
             RegionOfInterest mask;

            /*!
            *  \brief Step 1: Grayscale
            *  First of all, we want to make the image into a grayscale one; only one color channel. 
            *  This will help us with the identification of edges and corners.
            *  \param 
            */
            void grayscal (const Mat& frame_to_compute, Mat& frame_computed);

            /*!
            *  \brief Step 2: Gaussian Blur
            *  Adding Gaussian noise to an image, it very useful as it smooths the interpolation between the pixels and is a way 
            *  to super-pass noise and spurious gradients. Higher the kernel, the more blur the outcome image will be.
            *  https://pyimagesearch.com/2021/04/28/opencv-smoothing-and-blurring/
            *  \param 
            */
            void gaussian_blur(const Mat& frame_to_compute, Mat& frame_computed);

            /*!
            *  \brief
            *  \param 
            */
            void median_blur(const Mat& frame_to_compute, Mat& frame_computed);

            /*!
            *  \brief Step 3: Canny Edge Detection
            *  Canny Edge Detection offers a way to detect the boundaries of an image. This is done through the gradients of the image.
            *  The latter is nothing more that a function, where the brightness of each pixel corresponds to the strength of the gradient .
            *  We will find the edges by tracing the pixels that follow the strongest gradients! As in general the gradients show how rapidly 
            *  a function changes, an intense density change between the pixels will indicate an edge.
            *  \param 
            */
            void canny_edge_detection(const Mat& frame_to_compute, Mat& frame_computed);

            /*!
            *  \brief
            *  \param 
            */
            void hough_lines( const Mat& frame_to_compute,Mat& frame_with_lines_drew);

            /*!
            *  \brief Compute angle angle between line and x-axis
            *  \param Two points in the line
            */
            double compute_angle_from_two_points (cv::Point point_a, cv::Point point_b) ;
    };
#endif