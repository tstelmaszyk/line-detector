#include "doctest.h"

#include <opencv2/core.hpp>

#include <vector>

#include "ImageSink/NullImageSink.h"
#include "LaneConfig/LaneConfig.h"
#include "PerspectiveView/PerspectiveView.h"
#include "VideoCaracteristics/VideoCaracteristics.h"

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

TEST_CASE( "avec bottom_y_ratio=1.0 et bottom_width_ratio=0.5, le quad source touche les coins bas de l'image" )
{
  ::cv::Mat img( 720, 1280, CV_8UC1, ::cv::Scalar( 0 ) );
  VideoCaracteristics video( img );
  LaneConfig config;
  config.src_bottom_y_ratio = 1.0f;
  config.src_bottom_width_ratio = 0.5f;
  NullImageSink sink;
  PerspectiveView perspective( video, config, sink );

  const ::std::vector< ::cv::Point2f >& quad = perspective.source_quad();

  REQUIRE( quad.size() == 4 );
  CHECK( quad[ 2 ].x == doctest::Approx( 1280.0f ) );  // bas droit
  CHECK( quad[ 2 ].y == doctest::Approx( 720.0f ) );
  CHECK( quad[ 3 ].x == doctest::Approx( 0.0f ) );     // bas gauche
  CHECK( quad[ 3 ].y == doctest::Approx( 720.0f ) );
}

TEST_CASE( "le bord bas du quad source suit src_bottom_y_ratio et src_bottom_width_ratio" )
{
  ::cv::Mat img( 720, 1280, CV_8UC1, ::cv::Scalar( 0 ) );
  VideoCaracteristics video( img );
  LaneConfig config;
  config.src_bottom_y_ratio = 0.6f;
  config.src_bottom_width_ratio = 0.3f;
  NullImageSink sink;
  PerspectiveView perspective( video, config, sink );

  const ::std::vector< ::cv::Point2f >& quad = perspective.source_quad();

  REQUIRE( quad.size() == 4 );
  // center_x = 640, bottom_y = 0.6*720 = 432, bottom_half = 0.3*1280 = 384.
  CHECK( quad[ 2 ].x == doctest::Approx( 1024.0f ) );  // bas droit : 640 + 384
  CHECK( quad[ 2 ].y == doctest::Approx( 432.0f ) );
  CHECK( quad[ 3 ].x == doctest::Approx( 256.0f ) );   // bas gauche : 640 - 384
  CHECK( quad[ 3 ].y == doctest::Approx( 432.0f ) );
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
