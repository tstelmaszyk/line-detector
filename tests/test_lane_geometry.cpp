#include "doctest.h"

#include <opencv2/core.hpp>

#include "LaneConfig.h"
#include "LaneGeometry.h"
#include "LaneModel.h"
#include "VideoCaracteristics.h"

static LanePolynomial straight( double x )
{
  LanePolynomial poly;
  poly.quadratic_coefficient = 0;
  poly.linear_coefficient = 0;
  poly.constant_coefficient = x;
  poly.valid = true;
  return poly;
}

TEST_CASE( "voie symetrique -> offset nul" )
{
  ::cv::Mat ref( 720, 1280, CV_8UC3 );
  VideoCaracteristics video( ref );
  LaneConfig config;

  LaneModel model;
  model.left = straight( 440 );
  model.right = straight( 840 );

  const LaneModel result = LaneGeometry::compute( model, video, config );

  REQUIRE( result.lane_detected );
  CHECK( result.lateral_offset_px == doctest::Approx( 0.0 ) );
  CHECK( result.normalized_offset == doctest::Approx( 0.0 ) );
  CHECK_FALSE( result.reconstructed );
}

TEST_CASE( "centre de voie a droite du centre image -> offset negatif" )
{
  ::cv::Mat ref( 720, 1280, CV_8UC3 );
  VideoCaracteristics video( ref );
  LaneConfig config;

  LaneModel model;
  model.left = straight( 500 );
  model.right = straight( 900 );

  const LaneModel result = LaneGeometry::compute( model, video, config );

  REQUIRE( result.lane_detected );
  CHECK( result.lateral_offset_px < 0.0 );
  CHECK( result.normalized_offset < 0.0 );
  CHECK( result.lateral_offset_px == doctest::Approx( -60.0 ) );
  CHECK( result.normalized_offset == doctest::Approx( -0.3 ) );
  CHECK_FALSE( result.reconstructed );
}

TEST_CASE( "un seul cote valide + default_lane_width_px -> reconstruction" )
{
  ::cv::Mat ref( 720, 1280, CV_8UC3 );
  VideoCaracteristics video( ref );
  LaneConfig config;
  config.default_lane_width_px = 400.0;

  LaneModel model;
  model.left = straight( 440 );

  const LaneModel result = LaneGeometry::compute( model, video, config );

  REQUIRE( result.lane_detected );
  CHECK( result.right.valid );
  CHECK( result.right.constant_coefficient == doctest::Approx( 840.0 ) );
  CHECK( result.reconstructed );
}

TEST_CASE( "aucun cote valide -> lane_detected faux" )
{
  ::cv::Mat ref( 720, 1280, CV_8UC3 );
  VideoCaracteristics video( ref );
  LaneConfig config;

  LaneModel model;

  const LaneModel result = LaneGeometry::compute( model, video, config );

  CHECK_FALSE( result.lane_detected );
}
