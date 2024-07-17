#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "DetectLines.h"
#include "RegionOfInterest.h"

using namespace cv;


int main(int argc, char** argv )
{
    cv::Mat image;
    image = imread("/home/tsvk/Documents/vacap/img_piste/IMG_0417.jpeg",IMREAD_COLOR);
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }

    cv::Mat image_out ;

    RegionOfInterest test(image);
    test.apply_mask(image);

    DetectLines detecteur(image);
    detecteur.draw_lines(image,image_out);

    namedWindow( "Detected Lines", cv::WINDOW_KEEPRATIO);
    imshow( "Detected Lines", image_out );
    waitKey(0);
    return 0;
}