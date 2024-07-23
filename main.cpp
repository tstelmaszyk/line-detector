#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "DetectLines.h"
#include "RegionOfInterest.h"
#include "VideoCaracteristics.h"

#include <iostream>
#include <sstream>
#include <new>
#include <string>
#include <sstream>



#define INPUT_WIDTH 3264
#define INPUT_HEIGHT 2464

#define DISPLAY_WIDTH 640
#define DISPLAY_HEIGHT 480

#define CAMERA_FRAMERATE 21/1
#define FLIP 2

using namespace cv;


int main(int argc, char** argv )
{
    cv::Mat image;
    cv::Mat image_camera;
    cv::Mat image_out ;

    int height(1080);
    int width(1920);
    cv::VideoCapture cap(0);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FOURCC, 0x21);
    cap.set(cv::CAP_PROP_FPS, 30);
    if (!cap.isOpened())
    {
        std::cout << "Unable to get video from the camera!" << std::endl;
        return -1;
    }
    std::cout << "Got here!" << std::endl;
    cap.read(image_camera);



    image = imread("/home/tsvk/Documents/vacap/img_piste/IMG_0417.jpeg",IMREAD_COLOR);
    //image_camera = imread("/home/tsvk/Documents/vacap/test.jpg",IMREAD_COLOR);
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }

    namedWindow( "Camera", cv::WINDOW_KEEPRATIO);
    imshow( "Camera", image_camera );

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