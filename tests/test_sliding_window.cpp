#include "doctest.h"

#include <opencv2/imgproc.hpp>

#include "ImageSink/NullImageSink.h"
#include "LaneConfig/LaneConfig.h"
#include "SlidingWindowSearch/SlidingWindowSearch.h"
#include "VideoCaracteristics/VideoCaracteristics.h"

TEST_CASE( "search separe deux bandes verticales en cotes gauche/droite" )
{
  ::cv::Mat bev( 720, 1280, CV_8UC1, ::cv::Scalar( 0 ) );
  ::cv::rectangle( bev, ::cv::Rect( 300, 0, 12, 720 ), ::cv::Scalar( 255 ), ::cv::FILLED );
  ::cv::rectangle( bev, ::cv::Rect( 900, 0, 12, 720 ), ::cv::Scalar( 255 ), ::cv::FILLED );

  VideoCaracteristics video( bev );
  LaneConfig config;
  NullImageSink sink;
  SlidingWindowSearch search( video, config, sink );

  const LanePixels pixels = search.search( bev );

  REQUIRE( pixels.left.size() > 100 );
  REQUIRE( pixels.right.size() > 100 );

  double left_mean_x = 0;

  for ( const ::cv::Point& point : pixels.left )
    {
    left_mean_x += point.x;
    }

  left_mean_x /= pixels.left.size();

  double right_mean_x = 0;

  for ( const ::cv::Point& point : pixels.right )
    {
    right_mean_x += point.x;
    }

  right_mean_x /= pixels.right.size();

  CHECK( left_mean_x == doctest::Approx( 306 ).epsilon( 0.1 ) );
  CHECK( right_mean_x == doctest::Approx( 906 ).epsilon( 0.1 ) );
}
