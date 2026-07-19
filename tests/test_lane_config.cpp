#include "doctest.h"

#include "LaneConfig.h"

TEST_CASE( "LaneConfig fournit des valeurs par defaut saines" )
{
  LaneConfig config;

  CHECK( config.window_count > 0 );
  CHECK( config.window_margin > 0 );
  CHECK( config.window_min_pix > 0 );
  CHECK( config.white_threshold > 0 );
  CHECK( config.src_top_width_ratio > 0.0f );
  CHECK( config.src_top_y_ratio > 0.0f );
  CHECK( config.src_top_y_ratio < 1.0f );
  CHECK( config.default_lane_width_px == doctest::Approx( 0.0 ) );
}
