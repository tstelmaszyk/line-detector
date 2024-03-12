#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace cv;
int main(int argc, char** argv )
{
    Mat image;
    image = imread("/home/tsvk/Documents/vacap/CarND-LaneLines-P1/test_images/solidWhiteRight.jpg",IMREAD_COLOR);
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    cv::Mat output;
    cv::inRange(image, cv::Scalar(200, 200, 200), cv::Scalar(255, 255, 255), output);
    cv::imshow("output", output);
    //namedWindow("Display Image", WINDOW_AUTOSIZE );
    //imshow("Display Image", output);
    waitKey(0);
    return 0;
}