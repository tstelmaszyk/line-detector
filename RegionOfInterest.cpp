
#include "RegionOfInterest.h"

RegionOfInterest::RegionOfInterest(const VideoCaracteristics& video_properties):    video_properties(video_properties),
                                                                                    mask_to_apply(Mat::zeros(video_properties.image_size, CV_8UC1))
    {
        compute_trapeze_point_coordinates(mask_vertex_pts);
        fillPoly(this->mask_to_apply, mask_vertex_pts, Scalar(255, 255, 255), cv::LINE_8, 0);
    }


void RegionOfInterest::apply_mask(cv::Mat &frame_to_mask)
    {
        cv::Mat masked_frame = Mat::zeros(frame_to_mask.size(), CV_8UC3);
        bitwise_and(frame_to_mask, frame_to_mask, masked_frame, this->mask_to_apply);
        frame_to_mask = masked_frame;
    }

void RegionOfInterest::compute_trapeze_point_coordinates(std::vector<Point> &pts)
    {
        const cv::Point left_top(       video_properties.width_pixel/2-video_properties.width_pixel/10,
                                        video_properties.height_pixel/2 - video_properties.height_pixel/12) ;
        const cv::Point left_bottom(    0,
                                        video_properties.height_pixel) ;
        const cv::Point right_top(      video_properties.width_pixel/2+video_properties.width_pixel/10,
                                        video_properties.height_pixel/2 - video_properties.height_pixel/12);
        const cv::Point right_bottom(   video_properties.width_pixel,
                                        video_properties.height_pixel);
        pts.push_back(left_top);
        pts.push_back(right_top);
        pts.push_back(right_bottom);
        pts.push_back(left_bottom);
    }
