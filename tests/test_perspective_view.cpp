#include "doctest.h"

#include <opencv2/core.hpp>

#include <vector>

#include "LaneConfig.h"
#include "NullImageSink.h"
#include "PerspectiveView.h"
#include "VideoCaracteristics.h"

TEST_CASE( "to_bev produit une image a la taille BEV" )
{
  ::cv::Mat img( 720, 1280, CV_8UC1, ::cv::Scalar( 0 ) );
  VideoCaracteristics video( img );
  LaneConfig config;
  NullImageSink sink;
  PerspectiveView perspective( video, config, sink );

  ::cv::Mat bev;
  perspective.to_bev( img, bev );

  CHECK( bev.size() == perspective.bev_size() );
}

TEST_CASE( "warp_back apres to_bev revient a la taille d'origine" )
{
  ::cv::Mat img( 720, 1280, CV_8UC1, ::cv::Scalar( 0 ) );
  VideoCaracteristics video( img );
  LaneConfig config;
  NullImageSink sink;
  PerspectiveView perspective( video, config, sink );

  ::cv::Mat bev;
  ::cv::Mat back;
  perspective.to_bev( img, bev );
  perspective.warp_back( bev, back );

  CHECK( back.size() == img.size() );
}

TEST_CASE( "le quad source a 4 sommets dans l'image" )
{
  ::cv::Mat img( 720, 1280, CV_8UC1, ::cv::Scalar( 0 ) );
  VideoCaracteristics video( img );
  LaneConfig config;
  NullImageSink sink;
  PerspectiveView perspective( video, config, sink );

  const ::std::vector< ::cv::Point2f >& quad = perspective.source_quad();

  REQUIRE( quad.size() == 4 );

  for ( const ::cv::Point2f& corner : quad )
    {
    CHECK( corner.x >= 0.0f );
    CHECK( corner.x <= 1280.0f );
    CHECK( corner.y >= 0.0f );
    CHECK( corner.y <= 720.0f );
    }
}
