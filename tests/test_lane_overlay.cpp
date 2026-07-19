#include "doctest.h"

#include <opencv2/core.hpp>

#include "LaneConfig.h"
#include "LaneModel.h"
#include "LaneOverlay.h"
#include "NullImageSink.h"
#include "PerspectiveView.h"
#include "VideoCaracteristics.h"

TEST_CASE( "render preserve taille/type et modifie des pixels quand la voie est detectee" )
{
  ::cv::Mat img( 720, 1280, CV_8UC3, ::cv::Scalar( 100, 100, 100 ) );
  VideoCaracteristics video( img );
  LaneConfig config;
  NullImageSink sink;
  PerspectiveView perspective( video, config, sink );
  LaneOverlay overlay( perspective, sink );

  LaneModel model;
  model.left.quadratic_coefficient = 0;
  model.left.linear_coefficient = 0;
  model.left.constant_coefficient = 400;
  model.left.valid = true;
  model.right.quadratic_coefficient = 0;
  model.right.linear_coefficient = 0;
  model.right.constant_coefficient = 800;
  model.right.valid = true;
  model.lane_detected = true;

  ::cv::Mat out;
  overlay.render( img, model, out );

  CHECK( out.size() == img.size() );
  CHECK( out.type() == img.type() );
  CHECK( ::cv::countNonZero( out.reshape( 1 ) != img.reshape( 1 ) ) > 0 );
}

TEST_CASE( "render sans voie detectee renvoie une copie inchangee" )
{
  ::cv::Mat img( 720, 1280, CV_8UC3, ::cv::Scalar( 100, 100, 100 ) );
  VideoCaracteristics video( img );
  LaneConfig config;
  NullImageSink sink;
  PerspectiveView perspective( video, config, sink );
  LaneOverlay overlay( perspective, sink );

  LaneModel model;

  ::cv::Mat out;
  overlay.render( img, model, out );

  CHECK( 0 == ::cv::countNonZero( out.reshape( 1 ) != img.reshape( 1 ) ) );
}
