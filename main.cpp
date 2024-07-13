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
    cv::Mat prov ;
    bilateralFilter(frame_to_compute,prov,5,250,250);
    cv::cvtColor(prov,frame_computed,COLOR_RGB2GRAY);
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
    cv::Size kernel_size(35,35) ;
    //GaussianBlur(frame_to_compute,frame_computed,kernel_size,0);
    medianBlur(frame_to_compute,frame_computed,13);
        //bilateralFilter(frame_to_compute,frame_computed,5,250,250);
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
    const double low_threshold = 100;
    const double high_threshold = 200;
    cv::Canny(frame_to_compute,frame_computed,low_threshold,high_threshold,3,false); 
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
    const DimensionImage width_pix ;
    const DimensionImage height_pix;
    std::vector<Point> mask_vertex_pts;
    cv::Mat mask_to_apply  ;
        
 public:
    RegionOfInterest(const Mat& reference_frame):   width_pix(reference_frame.size().width),
                                                    height_pix(reference_frame.size().height),
                                                    mask_to_apply(Mat::zeros(reference_frame.size(), CV_8UC1))
    {
        compute_trapeze_point_coordinates(mask_vertex_pts);
        fillPoly(this->mask_to_apply, mask_vertex_pts, Scalar(255, 255, 255), cv::LINE_8, 0),0;
    };

    /*!
     *  \brief Mask is applied to the frame sent
     */
    void apply_mask(cv::Mat &frame_to_mask)
    {
        cv::Mat masked_frame = Mat::zeros(frame_to_mask.size(), CV_8UC3);
        bitwise_and(frame_to_mask, frame_to_mask, masked_frame, this->mask_to_apply);
        frame_to_mask = masked_frame;
    }
private: 
    /*!
     *  \brief compute_point_coordinates
     *  (0,0) -- x
     *  |
     *  y
     *  Step 1 : we suppose top points are the same (middle of the picture)
     */
    void compute_trapeze_point_coordinates(std::vector<Point> &pts)
    {
        const cv::Point left_top(       width_pix/2-width_pix/10,
                                        height_pix/2 - height_pix/12) ;
        const cv::Point left_bottom(    0,
                                        height_pix) ;
        const cv::Point right_top(      width_pix/2+width_pix/10,
                                        height_pix/2 - height_pix/12);
        const cv::Point right_bottom(   width_pix,
                                        height_pix);
        pts.push_back(left_top);
        pts.push_back(right_top);
        pts.push_back(right_bottom);
        pts.push_back(left_bottom);
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
    //image = imread("/home/tsvk/Documents/vacap/CarND-LaneLines-P1/test_images/solidYellowCurve.jpg",IMREAD_COLOR);

    image = imread("/home/tsvk/Documents/vacap/img_piste/IMG_0418.jpeg",IMREAD_COLOR);
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }

    cv::Mat output_gray;
    grayscal(image,output_gray);
    namedWindow( "output gray", cv::WINDOW_KEEPRATIO);
    cv::imshow("output gray", output_gray);

    cv::Mat output_gaussian_blur;
    gaussian_blur(output_gray,output_gaussian_blur);
    namedWindow( "Gaussian", cv::WINDOW_KEEPRATIO);
    cv::imshow("Gaussian", output_gaussian_blur);

    cv::Mat output_canny ;
    canny_edge_detection(output_gaussian_blur,output_canny);
    namedWindow( "Canny", cv::WINDOW_KEEPRATIO);
    cv::imshow("Canny",output_canny);

    RegionOfInterest test(output_canny);
    //test.apply_mask(output_canny);

    std::vector<Vec4i> lines;
    int min_line_height = (int)output_canny.size().height / 5 ; 
    std::cout << min_line_height ; 
    HoughLinesP( output_canny, lines, 1, CV_PI/180, 15, min_line_height, 80 ); // A tester avec ttes les qualites dimages!!
    
    //Output vector of lines. Each line is represented by a 4-element vector x_1, y_1, x_2, y_2), 
    //where x_1,y_1)and x_2, y_2)are the ending points of each detected line segment.
    
    double angle = 0.0 ;
    std::vector<Point> pts;
    for( size_t i = 0; i < lines.size(); i++ )
    {
        //if (lines[i][0]<output_canny.size().width/2) //On travaille sur la moitié gauche de l'image 
        //{
            angle = compute_angle_from_two_points(  Point(lines[i][0], lines[i][1]),
                                                    Point(lines[i][2], lines[i][3]));

            std::cout<<angle<<std::endl;                                        
            if (angle > 40)
            {
            pts.push_back(Point(lines[i][0], lines[i][1]));
            pts.push_back(Point(lines[i][2], lines[i][3]));
            line( image, Point(lines[i][0], lines[i][1]),
            Point( lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
            //std::cout<<angle<<std::endl;
            }
        //}
    }   

    namedWindow( "Detected Lines 2", cv::WINDOW_KEEPRATIO);
    imshow( "Detected Lines 2", image );
    waitKey(0);
    return 0;
}