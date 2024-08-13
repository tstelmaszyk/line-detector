#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <libcamera/libcamera.h>

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



    image = imread("/home/tsvk/Documents/line-detector/img_piste/img2.jpg",IMREAD_COLOR);
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }


    VideoCaracteristics video_properties (image);
    DetectLines detecteur(video_properties);
    detecteur.draw_lines(image,image_out);

    namedWindow( "Detected Lines", cv::WINDOW_AUTOSIZE);
    imshow( "Detected Lines", image_out );
    waitKey(0);
    return 0;
}