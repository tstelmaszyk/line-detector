#include "doctest.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <cmath>

#include "DetectLines.h"
#include "DiskImageSink.h"
#include "LaneConfig.h"
#include "NullImageSink.h"
#include "VideoCaracteristics.h"
#include "test_support.h"

namespace
{

const int SINK_TEST_WIDTH = 1280;   ///< Largeur de la frame de test.
const int SINK_TEST_HEIGHT = 720;   ///< Hauteur de la frame de test.
const int SINK_LEFT_LINE_X = 440;   ///< Abscisse de la ligne gauche.
const int SINK_RIGHT_LINE_X = 840;  ///< Abscisse de la ligne droite.
const int SINK_LINE_THICKNESS = 14;    ///< Epaisseur des lignes tracees.
const int SINK_BACKGROUND_GRAY = 110;  ///< Gris du fond.
const double SINK_LANE_WIDTH_RATIO = 0.5;  ///< Largeur de voie par defaut.
const double SINK_OFFSET_TOLERANCE = 0.15;  ///< Tolerance sur l'offset normalise.

/// @brief Construit une image de voie synthetique symetrique.
::cv::Mat make_lane_frame()
  {
  const ::cv::Scalar background( SINK_BACKGROUND_GRAY, SINK_BACKGROUND_GRAY, SINK_BACKGROUND_GRAY );
  ::cv::Mat frame( SINK_TEST_HEIGHT, SINK_TEST_WIDTH, CV_8UC3, background );
  const ::cv::Scalar line_color( 255, 255, 255 );
  const ::cv::Point left_bottom( SINK_LEFT_LINE_X, SINK_TEST_HEIGHT - 1 );
  const ::cv::Point left_top( SINK_LEFT_LINE_X, SINK_TEST_HEIGHT / 2 );
  const ::cv::Point right_bottom( SINK_RIGHT_LINE_X, SINK_TEST_HEIGHT - 1 );
  const ::cv::Point right_top( SINK_RIGHT_LINE_X, SINK_TEST_HEIGHT / 2 );
  ::cv::line( frame, left_bottom, left_top, line_color, SINK_LINE_THICKNESS );
  ::cv::line( frame, right_bottom, right_top, line_color, SINK_LINE_THICKNESS );
  return frame;
  }

} // namespace

TEST_CASE( "NullImageSink : is_enabled renvoie false" )
{
  NullImageSink sink;
  const bool enabled = sink.is_enabled();

  CHECK( false == enabled );
}

TEST_CASE( "DiskImageSink : is_enabled renvoie true" )
{
  DiskImageSink sink( test_temp_dir() );
  const bool enabled = sink.is_enabled();

  CHECK( true == enabled );
}

TEST_CASE( "DetectLines::draw_lines : meme LaneModel avec un NullImageSink" )
{
  const ::cv::Mat frame = make_lane_frame();
  VideoCaracteristics video( frame );
  LaneConfig config;
  config.default_lane_width_px = SINK_TEST_WIDTH * SINK_LANE_WIDTH_RATIO;
  NullImageSink sink;
  const DetectLines detector( video, config, sink );

  ::cv::Mat out;
  const LaneModel model = detector.draw_lines( frame, out );

  REQUIRE( model.lane_detected );
  CHECK( ::std::abs( model.normalized_offset ) < SINK_OFFSET_TOLERANCE );
}
