#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace cv;

void polyfit(const Mat& src_x, const Mat& src_y, Mat& dst, int order)
{
    CV_Assert((src_x.rows>0)&&(src_y.rows>0)&&(src_x.cols==1)&&(src_y.cols==1)
            &&(dst.cols==1)&&(dst.rows==(order+1))&&(order>=1));
    Mat X;
    X = Mat::zeros(src_x.rows, order+1,CV_32FC1);
    Mat copy;
    for(int i = 0; i <=order;i++)
    {
        copy = src_x.clone();
        pow(copy,i,copy);
        Mat M1 = X.col(i);
        copy.col(0).copyTo(M1);
    }
    Mat X_t, X_inv;
    transpose(X,X_t);
    Mat temp = X_t*X;
    Mat temp2;
    invert (temp,temp2);
    Mat temp3 = temp2*X_t;
    Mat W = temp3*src_y;
    W.copyTo(dst);
}

struct RGBColor
{
    u_int8_t red ;
    u_int8_t green ;
    u_int8_t blue ;
    RGBColor(u_int8_t r, u_int8_t g, u_int8_t b):red(r) , green(g) , blue (b) {}
};
typedef RGBColor RGBColor;

struct Coordinates2D {
    float x;
    float y; 
    Coordinates2D(unsigned x, unsigned y):x(x),y(y){}
};
typedef Coordinates2D Coordinates2D;

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
    const RGBColor min_color(200,200,200);
    const RGBColor max_color(255, 255, 255);
    cv::inRange(image, 
                cv::Scalar(min_color.red, min_color.green, min_color.blue), 
                cv::Scalar(max_color.red, max_color.green, max_color.blue), 
                output);
    cv::imshow("output", output);



    Coordinates2D left_bottom(100.0,539.0);
    Coordinates2D right_bottom(950.0,539.0);
    Coordinates2D apex(480, 290);
    Mat x_left = (Mat_<float>(2,1) << left_bottom.x, apex.x);
    Mat y_left = (Mat_<float>(2,1) << left_bottom.y, apex.y);
    Mat fit_left = (Mat_<float>(2,1) << 0.0, 0.0);
    polyfit(x_left,y_left,fit_left,1);

    std::cout << "out :" << fit_left.row(0) ;
    std::cout << "out :" << fit_left.row(1) ;
    Mat right = (Mat_<double>(2,1) << right_bottom.x, right_bottom.y);
    Mat top = (Mat_<double>(2,1) << apex.x, apex.y);
    

    waitKey(0);


    return 0;
}