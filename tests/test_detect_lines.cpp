#include "doctest.h"

#include <opencv2/imgproc.hpp>

#include "DetectLines.h"
#include "LaneConfig.h"
#include "NullImageSink.h"
#include "VideoCaracteristics.h"

static ::cv::Mat make_lane_image( int width, int height, int left_x, int right_x )
{
  ::cv::Mat img( height, width, CV_8UC3, ::cv::Scalar( 110, 110, 110 ) );
  ::cv::line( img, { left_x, height - 1 }, { left_x, height / 2 }, ::cv::Scalar( 255, 255, 255 ), 14 );
  ::cv::line( img, { right_x, height - 1 }, { right_x, height / 2 }, ::cv::Scalar( 255, 255, 255 ), 14 );
  return img;
}

TEST_CASE( "draw_lines renvoie une image a la taille d'origine" )
{
  ::cv::Mat img = make_lane_image( 1280, 720, 440, 840 );
  VideoCaracteristics video( img );
  LaneConfig config;
  config.default_lane_width_px = 640.0;
  NullImageSink sink;
  DetectLines detector( video, config, sink );

  ::cv::Mat out;
  detector.draw_lines( img, out );

  CHECK( out.size() == img.size() );
  CHECK( out.type() == img.type() );
}
