#include "doctest.h"

#include <opencv2/imgproc.hpp>

#include <cmath>

#include "DetectLines/DetectLines.h"
#include "ImageSink/NullImageSink.h"
#include "LaneConfig/LaneConfig.h"
#include "LanePolynomial/LanePolynomial.h"
#include "SlidingWindowSearch/SlidingWindowSearch.h"
#include "VideoCaracteristics/VideoCaracteristics.h"

static ::cv::Mat make_lane_image( int width, int height, int left_x, int right_x )
{
  ::cv::Mat img( height, width, CV_8UC3, ::cv::Scalar( 110, 110, 110 ) );
  ::cv::line( img, { left_x, height - 1 }, { left_x, height / 2 }, ::cv::Scalar( 255, 255, 255 ), 14 );
  ::cv::line( img, { right_x, height - 1 }, { right_x, height / 2 }, ::cv::Scalar( 255, 255, 255 ), 14 );
  return img;
}

/// @brief Trapeze BEV attendu par make_lane_image (lignes de mi-hauteur au bas,
/// plein cadre) : independant de la calibration camera reelle de LaneConfig.
static void configure_synthetic_bev_trapezoid( LaneConfig& p_config )
{
  p_config.src_top_y_ratio = 0.45f;
  p_config.src_top_width_ratio = 0.18f;
  p_config.src_bottom_y_ratio = 1.0f;
  p_config.src_bottom_width_ratio = 0.5f;
}

TEST_CASE( "pipeline complet : voie symetrique -> offset proche de zero" )
{
  const int width = 1280;
  const int height = 720;
  ::cv::Mat img = make_lane_image( width, height, 440, 840 );
  VideoCaracteristics video( img );
  LaneConfig config;
  config.default_lane_width_px = width * 0.5;
  configure_synthetic_bev_trapezoid( config );
  NullImageSink sink;
  DetectLines detector( video, config, sink );

  const LaneModel model = detector.compute( img );

  REQUIRE( model.lane_detected );
  CHECK( ::std::abs( model.normalized_offset ) < 0.15 );
}

TEST_CASE( "pipeline complet : voie decalee a droite -> offset negatif" )
{
  const int width = 1280;
  const int height = 720;
  ::cv::Mat img = make_lane_image( width, height, 540, 940 );
  VideoCaracteristics video( img );
  LaneConfig config;
  config.default_lane_width_px = width * 0.5;
  configure_synthetic_bev_trapezoid( config );
  NullImageSink sink;
  DetectLines detector( video, config, sink );

  const LaneModel model = detector.compute( img );

  REQUIRE( model.lane_detected );
  CHECK( model.normalized_offset < 0.0 );
}

TEST_CASE( "recherche + fit sur voie courbe -> terme quadratique non nul" )
{
  const int width = 1280;
  const int height = 720;
  const double curvature = 3e-4;
  const int left_base = 300;
  const int right_base = 900;
  const int stripe_half = 8;

  ::cv::Mat bev( height, width, CV_8UC1, ::cv::Scalar( 0 ) );

  for ( int y = 0; y < height; ++y )
    {
    const double y_from_bottom = static_cast< double >( height - 1 - y );
    const int x_left = ::cvRound( left_base + ( curvature * y_from_bottom * y_from_bottom ) );
    const int x_right = ::cvRound( right_base + ( curvature * y_from_bottom * y_from_bottom ) );

    for ( int dx = -stripe_half; dx <= stripe_half; ++dx )
      {
      if ( ( x_left + dx >= 0 ) && ( x_left + dx < width ) )
        {
        bev.at< uchar >( y, x_left + dx ) = 255;
        }

      if ( ( x_right + dx >= 0 ) && ( x_right + dx < width ) )
        {
        bev.at< uchar >( y, x_right + dx ) = 255;
        }
      }
    }

  ::cv::Mat ref( height, width, CV_8UC3 );
  VideoCaracteristics video( ref );
  LaneConfig config;
  config.window_count = 9;
  config.window_margin = 80;
  config.window_min_pix = 5;
  NullImageSink sink;
  SlidingWindowSearch search( video, config, sink );

  const LanePixels pixels = search.search( bev );
  const LanePolynomial fit_left = LanePolynomial::fit( pixels.left, config.window_min_pix );
  const LanePolynomial fit_right = LanePolynomial::fit( pixels.right, config.window_min_pix );

  REQUIRE( fit_left.valid );
  REQUIRE( fit_right.valid );
  CHECK( fit_left.quadratic_coefficient > 1e-4 );
  CHECK( fit_right.quadratic_coefficient > 1e-4 );
}
