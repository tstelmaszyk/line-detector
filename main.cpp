#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "DetectLines.h"
#include "RegionOfInterest.h"
#include "VideoCaracteristics.h"

using namespace cv;


int main(int argc, char** argv )
{
    cv::Mat image;
    cv::Mat image_out ;

    image = imread("/home/tsvk/Documents/vacap/img_piste/IMG_0417.jpeg",IMREAD_COLOR);
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    VideoCaracteristics video_properties (image);
    RegionOfInterest test(video_properties);
    test.apply_mask(image);

    DetectLines detecteur(video_properties);
    detecteur.draw_lines(image,image_out);

    namedWindow( "Detected Lines", cv::WINDOW_KEEPRATIO);
    imshow( "Detected Lines", image_out );
    waitKey(0);
    return 0;
}