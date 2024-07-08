#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <math.h>

using namespace cv;

typedef uint16_t DimensionImage ;

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
    const double low_threshold = 200;
    const double high_threshold = 300;
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
class RegionOfInterest
{
    private :
    const 
    DimensionImage width_pix ;
    DimensionImage height_pix;

    std::vector<Point> pts;

    cv::Point left_top{} ;
    cv::Point left_bottom{};
    cv::Point right_top{} ;
    cv::Point right_bottom{};
 public:
    RegionOfInterest(const Mat& reference_frame):   width_pix(reference_frame.size().width),
                                                    height_pix(reference_frame.size().height)
    {
        compute_point_coordinates();
        pts.push_back(left_top);
        pts.push_back(right_top);
        pts.push_back(right_bottom);
        pts.push_back(left_bottom);
    };
    void draw(cv::Mat img)
    {
        polylines( img, pts, true, Scalar(0,0,0), 2, 8, 0);
        cv::imshow("test",img);
    }

    void mask(cv::Mat &img)
    {
        cv::Mat final = Mat::zeros(img.size(), CV_8UC3);
        cv::Mat mask = Mat::zeros(img.size(), CV_8UC1);

        fillPoly(mask, pts, Scalar(255, 255, 255), 8, 0);
        bitwise_and(img, img, final, mask);
        imshow("Mask", mask);
        imshow("Result", final);
        img = final;
    }
private: 
    /*!
     *  \brief compute_point_coordinates
     *  (0,0) -- x
     *  |
     *  y
     *  Step 1 : we suppose top points are the same (middle of the picture)
     */
    void compute_point_coordinates()
    {
        left_top.x = width_pix/2-width_pix/10;
        right_top.x = width_pix/2+width_pix/10;
        
        left_top.y = height_pix/2 - height_pix/6;
        right_top.y = height_pix/2 - height_pix/6;

        left_bottom.y = height_pix ;
        right_bottom.y = height_pix ;

        left_bottom.x = 0;
        right_bottom.x = width_pix;
    }
};


    /*!
     *  \brief Compute angle angle between line and x-axis
     * 
     *  \param Two points in the line
     */
double compute_angle_from_two_points (cv::Point point_a, cv::Point point_b) 
{
    double cos_angle = 0.0 ;
    double angle = 0.0 ;
    const double rad_to_degree = 180/CV_PI ;
    cos_angle = (point_b.x-point_a.x) / sqrt( (point_b.x-point_a.x)*(point_b.x-point_a.x) + (point_b.y-point_a.y)*(point_b.y-point_a.y) );
    angle = acos(cos_angle) * rad_to_degree;
    return angle ;
}


int main(int argc, char** argv )
{
    cv::Mat image;
    cv::Mat image2;
    image = imread("/home/tsvk/Documents/vacap/CarND-LaneLines-P1/test_images/solidWhiteCurve.jpg",IMREAD_COLOR);
    image2 = imread("/home/tsvk/Documents/vacap/CarND-LaneLines-P1/test_images/solidWhiteCurve.jpg",IMREAD_COLOR);
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }

    cv::Mat output_gray;
    grayscal(image,output_gray);
    cv::imshow("output gray", output_gray);

    cv::Mat output_gaussian_blur;
    gaussian_blur(output_gray,output_gaussian_blur);
    cv::imshow("Gaussian", output_gaussian_blur);

    cv::Mat output_canny ;
    canny_edge_detection(output_gaussian_blur,output_canny);
    cv::imshow("Canny",output_canny);

    RegionOfInterest test(output_canny);
    test.draw(output_canny);
    test.mask(output_canny);

    std::vector<Vec4i> lines;
    HoughLinesP( output_canny, lines, 1, CV_PI/180, 15, 10, 40 );
    for( size_t i = 0; i < lines.size(); i++ )
    {
        if (lines[i][0]<output_canny.size().width) //On travaille sur la moitié gauche de l'image 
        {
            line( image, Point(lines[i][0], lines[i][1]),
            Point( lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
        }
    }
    namedWindow( "Detected Lines", 1 );
    imshow( "Detected Lines", image );

    //Output vector of lines. Each line is represented by a 4-element vector x_1, y_1, x_2, y_2), 
    //where x_1,y_1)and x_2, y_2)are the ending points of each detected line segment.
    
    double angle = 0.0 ;
    std::vector<Point> pts;
    for( size_t i = 0; i < lines.size(); i++ )
    {
        if (lines[i][0]>output_canny.size().width/2) //On travaille sur la moitié droite de l'image 
        {
            angle = compute_angle_from_two_points(  Point(lines[i][0], lines[i][1]),
                                                    Point(lines[i][2], lines[i][3]));
            std::cout<<angle << std::endl ;
            if (angle > 15)
            {
            pts.push_back(Point(lines[i][0], lines[i][1]));
            pts.push_back(Point(lines[i][2], lines[i][3]));
            }
        }
    }   

    cv::polylines(image2,pts,1,(0,255,255));
    namedWindow( "Detected Lines 2", 1 );
    imshow( "Detected Lines 2", image2 );
    waitKey(0);
    return 0;
}