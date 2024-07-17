
#include "RegionOfInterest.h"

RegionOfInterest::RegionOfInterest(const Mat& reference_frame): width_pix(reference_frame.size().width),
                                                                height_pix(reference_frame.size().height),
                                                                mask_to_apply(Mat::zeros(reference_frame.size(), CV_8UC1))
    {
        compute_trapeze_point_coordinates(mask_vertex_pts);
        fillPoly(this->mask_to_apply, mask_vertex_pts, Scalar(255, 255, 255), cv::LINE_8, 0),0;
    }


void RegionOfInterest::apply_mask(cv::Mat &frame_to_mask)
    {
        cv::Mat masked_frame = Mat::zeros(frame_to_mask.size(), CV_8UC3);
        bitwise_and(frame_to_mask, frame_to_mask, masked_frame, this->mask_to_apply);
        frame_to_mask = masked_frame;
    }

void RegionOfInterest::compute_trapeze_point_coordinates(std::vector<Point> &pts)
    {
        const cv::Point left_top(       width_pix/2-width_pix/10,
                                        height_pix/2 - height_pix/12) ;
        const cv::Point left_bottom(    0,
                                        height_pix) ;
        const cv::Point right_top(      width_pix/2+width_pix/10,
                                        height_pix/2 - height_pix/12);
        const cv::Point right_bottom(   width_pix,
                                        height_pix);
        pts.push_back(left_top);
        pts.push_back(right_top);
        pts.push_back(right_bottom);
        pts.push_back(left_bottom);
    }
