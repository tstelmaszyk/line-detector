#pragma once

#include "ImageSink.h"
#include <string>

/*!
*  \brief Sink disque : ecrit l'image dans output_dir via cv::imwrite.
*/
class DiskImageSink : public ImageSink
{
    public:
        explicit DiskImageSink(std::string output_dir);
        bool save(const std::string& name, const cv::Mat& frame) override;

    private:
        std::string output_dir;
};
