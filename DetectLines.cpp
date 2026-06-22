#include "DetectLines.h"

#include <cmath>
#include <opencv2/imgproc.hpp>

DetectLines::DetectLines(const VideoCaracteristics& video_properties, ImageSink& debug_sink):    video_properties(video_properties),
                                                                                                debug_sink(debug_sink),
                                                                                                mask(video_properties, debug_sink)
    {
    };

void DetectLines::draw_lines (const Mat& frame_to_compute, Mat& frame_with_lines)
    {
        cv::Mat output_gray;
        grayscal(frame_to_compute,output_gray);
        debug_sink.save("debug_01_gray.jpg", output_gray);

        cv::Mat output_gaussian_blur;
        median_blur(output_gray,output_gaussian_blur);
        debug_sink.save("debug_02_blur.jpg", output_gaussian_blur);

        cv::Mat output_canny ;
        canny_edge_detection(output_gaussian_blur,output_canny);
        debug_sink.save("debug_03_canny.jpg", output_canny);

        frame_with_lines = frame_to_compute.clone() ;
        mask.apply_mask(output_canny);
        hough_lines(output_canny,frame_with_lines);
        
    }

void DetectLines::grayscal (const Mat& frame_to_compute, Mat& frame_computed)
    {
        cv::Mat prov ;
        bilateralFilter(frame_to_compute,prov,5,250,250);
        cv::cvtColor(prov,frame_computed,COLOR_RGB2GRAY);
    }

        
void DetectLines::gaussian_blur(const Mat& frame_to_compute, Mat& frame_computed)
    {
        const cv::Size kernel_size(35,35) ;
        GaussianBlur(frame_to_compute,frame_computed,kernel_size,0);
    }

void DetectLines::median_blur(const Mat& frame_to_compute, Mat& frame_computed)
    {
        medianBlur(frame_to_compute,frame_computed,13);
    }


void DetectLines::canny_edge_detection(const Mat& frame_to_compute, Mat& frame_computed)
    {
        const double low_threshold = 100;
        const double high_threshold = 200;
        cv::Canny(frame_to_compute,frame_computed,low_threshold,high_threshold,3,false); 
    }


void DetectLines::hough_lines( const Mat& frame_to_compute,Mat& frame_with_lines_drew)
    {
        std::vector<Vec4i> lines;
        double angle = 0.0 ;
        const double rho = 1 ;
        const double theta = CV_PI/180 ;
        const int threshold = 15 ;
        const double min_line_height =  video_properties.height_pixel / 5 ;  
        const double max_line_gap = 80 ;
        
        HoughLinesP(    frame_to_compute, 
                        lines, 
                        rho, 
                        theta, 
                        threshold, 
                        min_line_height, 
                        max_line_gap ); //Output vector of lines. Each line is represented by a 4-element vector x_1, y_1, x_2, y_2), 
                                        //where x_1,y_1)and x_2, y_2)are the ending points of each detected line segment.
        
        std::vector<Vec4i>::iterator iter = lines.begin();
        while (iter != lines.end())
            {
                angle = compute_angle_from_two_points(  Point((*iter)[0], (*iter)[1]),
                                                        Point((*iter)[2], (*iter)[3]));
                if (angle > 40)
                {
                line(   frame_with_lines_drew, 
                        Point((*iter)[0], (*iter)[1]),
                        Point((*iter)[2], (*iter)[3]), 
                        Scalar(0,0,255), 3, 8 );
                }
                else
                {
                    //lines.erase(iter); segmentation fault when empty ?
                }
                ++iter;
            }
    }

double DetectLines::compute_angle_from_two_points (cv::Point point_a, cv::Point point_b)
    {
        const double rad_to_degree = 180/CV_PI ;
        const double dx = point_b.x - point_a.x ;
        const double dy = point_b.y - point_a.y ;
        if (dx == 0.0 && dy == 0.0)
        {
            return 0.0 ; // segment degenere (deux points confondus) : angle indefini
        }
        return std::abs(std::atan2(dy, dx)) * rad_to_degree ;
    }