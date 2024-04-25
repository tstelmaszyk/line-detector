#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace cv;

void polyfit(const Mat& src_x, const Mat& src_y, Mat& dst, int order)
{
    // Sinon voir : https://docs.opencv.org/4.x/d6/d6e/group__imgproc__draw.html#gaa3c25f9fb764b6bef791bf034f6e26f5
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


    /*!
     *  \brief Step 1: Grayscale
     *
     *  First of all, we want to make the image into a grayscale one; only one color channel. 
     *  This will help us with the identification of edges and corners.
     *
     *  \param 
     */
void grayscal (const Mat& frame_to_compute, Mat& frame_computed){
    cv::cvtColor(frame_to_compute,frame_computed,COLOR_RGB2GRAY);
}

    /*!
     *  \brief Step 2: Gaussian Blur
     *
     *  Adding Gaussian noise to an image, it very useful as it smooths the interpolation between the pixels and is a way 
     *  to super-pass noise and spurious gradients. Higher the kernel, the more blur the outcome image will be.
     *
     *  \param 
     */
void gaussian_blur(const Mat& frame_to_compute, Mat& frame_computed){
    cv::Size kernel_size(5,5) ;
    GaussianBlur(frame_to_compute,frame_computed,(kernel_size,kernel_size),0);
}


    /*!
     *  \brief Step 3: Canny Edge Detection
     *
     *  Canny Edge Detection offers a way to detect the boundaries of an image. This is done through the gradients of the image.
     *  The latter is nothing more that a function, where the brightness of each pixel corresponds to the strength of the gradient .
     *  We will find the edges by tracing the pixels that follow the strongest gradients! As in general the gradients show how rapidly 
     *  a function changes, an intense density change between the pixels will indicate an edge.
     *
     *  \param 
     */
void canny_edge_detection(const Mat& frame_to_compute, Mat& frame_computed){
    double low_threshold = 200;
    double high_threshold = 300;
    cv::Canny(frame_to_compute,frame_computed,low_threshold,high_threshold); 
}

    /*!
     *  \brief Step 4: Mask a region of interest
     *
     *  In the above picture, there are some outliers; some edges from the other part of the road, from the landscape (mountains), etc. 
     *  As our camera will be fixed, we can put a mask upon the image and keep only these lines that are interesting for our task. 
     *  Thus, it will be very natural to draw a trapezium in order to keep only an area on where we should expect the road lines to be. 
     * 
     *  \param 
     */
void mask(const Mat& frame_to_compute, Mat& frame_computed){
    double low_threshold = 200;
    double high_threshold = 300;
    cv::Canny(frame_to_compute,frame_computed,low_threshold,high_threshold); 
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