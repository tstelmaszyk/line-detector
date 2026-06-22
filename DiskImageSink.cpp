#include "DiskImageSink.h"

#include <utility>
#include <opencv2/imgcodecs.hpp>

DiskImageSink::DiskImageSink(std::string output_dir): output_dir(std::move(output_dir))
    {
    }

bool DiskImageSink::save(const std::string& name, const cv::Mat& frame)
    {
        return cv::imwrite(output_dir + "/" + name, frame);
    }
