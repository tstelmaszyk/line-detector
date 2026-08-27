#include "doctest.h"

#include <opencv2/imgproc.hpp>

#include "ImageSink/NullImageSink.h"
#include "LaneConfig/LaneConfig.h"
#include "LaneMask/LaneMask.h"
#include "VideoCaracteristics/VideoCaracteristics.h"

TEST_CASE( "LaneMask isole une bande blanche et ignore la route grise unie" )
{
  ::cv::Mat bgr( 100, 100, CV_8UC3, ::cv::Scalar( 110, 110, 110 ) );
  ::cv::rectangle( bgr, ::cv::Rect( 45, 0, 10, 100 ), ::cv::Scalar( 255, 255, 255 ), ::cv::FILLED );

  VideoCaracteristics video( bgr );
  LaneConfig config;
  NullImageSink sink;
  LaneMask mask( video, config, sink );

  ::cv::Mat out;
  mask.compute( bgr, out );

  CHECK( out.type() == CV_8UC1 );
  CHECK( out.size() == bgr.size() );
  CHECK( out.at< uchar >( 50, 50 ) == 255 );
  CHECK( out.at< uchar >( 50, 5 ) == 0 );
}
