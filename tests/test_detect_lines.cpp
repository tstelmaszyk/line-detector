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

TEST_CASE( "render produit une image a la taille d'origine" )
{
  ::cv::Mat img = make_lane_image( 1280, 720, 440, 840 );
  VideoCaracteristics video( img );
  LaneConfig config;
  config.default_lane_width_px = 640.0;
  NullImageSink sink;
  DetectLines detector( video, config, sink );

  const LaneModel model = detector.compute( img );

  ::cv::Mat out;
  detector.render( img, model, out );

  CHECK( out.size() == img.size() );
  CHECK( out.type() == img.type() );
}

TEST_CASE( "render est reproductible et ne mute pas le modele" )
{
  ::cv::Mat img = make_lane_image( 1280, 720, 440, 840 );
  VideoCaracteristics video( img );
  LaneConfig config;
  config.default_lane_width_px = 640.0;
  NullImageSink sink;
  DetectLines detector( video, config, sink );

  const LaneModel model = detector.compute( img );
  const double offset_before = model.normalized_offset;

  ::cv::Mat first_output;
  ::cv::Mat second_output;
  detector.render( img, model, first_output );
  detector.render( img, model, second_output );

  // Deux rendus du meme modele sur la meme frame donnent la meme image.
  ::cv::Mat difference;
  ::cv::absdiff( first_output, second_output, difference );
  const double max_difference = ::cv::norm( difference, ::cv::NORM_INF );

  CHECK( 0.0 == max_difference );
  CHECK( offset_before == model.normalized_offset );
}

TEST_CASE( "render accepte le modele d'une autre frame" )
{
  // Propriete sans consommateur aujourd'hui : elle est le prerequis du lot
  // LaneTracker, qui dessinera un modele lisse sur la frame courante.
  ::cv::Mat first_frame = make_lane_image( 1280, 720, 440, 840 );
  ::cv::Mat second_frame = make_lane_image( 1280, 720, 500, 900 );
  VideoCaracteristics video( first_frame );
  LaneConfig config;
  config.default_lane_width_px = 640.0;
  NullImageSink sink;
  DetectLines detector( video, config, sink );

  const LaneModel first_model = detector.compute( first_frame );

  ::cv::Mat out;
  detector.render( second_frame, first_model, out );

  CHECK( out.size() == second_frame.size() );
  CHECK( out.type() == second_frame.type() );
}
