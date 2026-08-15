#include "doctest.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#include <memory>
#include <string>

#include "CaptureFrameSource.h"
#include "StillImageFrameSource.h"
#include "test_support.h"

namespace
{

const int TEST_IMAGE_WIDTH = 64;    ///< Largeur des images de test.
const int TEST_IMAGE_HEIGHT = 48;   ///< Hauteur des images de test.
const int TEST_GRAY_LEVEL = 120;    ///< Niveau de gris de remplissage.

const int TEST_VIDEO_FRAME_COUNT = 5;         ///< Frames ecrites dans la video de test.
const double TEST_VIDEO_FPS = 10.0;           ///< Cadence de la video de test.
const int TEST_VIDEO_GRAY_STEP = 20;          ///< Ecart de gris entre deux frames.

/// @brief Ecrit une image de test sur disque et rend son chemin.
::std::string write_test_image( const ::std::string& p_file_name )
  {
  const ::std::string full_path = test_temp_dir() + "/" + p_file_name;
  const ::cv::Scalar fill_color( TEST_GRAY_LEVEL, TEST_GRAY_LEVEL, TEST_GRAY_LEVEL );
  const ::cv::Mat image( TEST_IMAGE_HEIGHT, TEST_IMAGE_WIDTH, CV_8UC3, fill_color );
  const bool written = ::cv::imwrite( full_path, image );
  REQUIRE( written );
  return full_path;
  }

/// @brief Ecrit une petite video de test sur disque et rend son chemin.
::std::string write_test_video( const ::std::string& p_file_name )
  {
  const ::std::string full_path = test_temp_dir() + "/" + p_file_name;
  const int fourcc = ::cv::VideoWriter::fourcc( 'M', 'J', 'P', 'G' );
  const ::cv::Size frame_size( TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT );
  ::cv::VideoWriter writer( full_path, fourcc, TEST_VIDEO_FPS, frame_size );
  const bool opened = writer.isOpened();
  REQUIRE( true == opened );

  for ( int frame_index = 0; frame_index < TEST_VIDEO_FRAME_COUNT; ++frame_index )
    {
    const int gray_level = TEST_GRAY_LEVEL + ( frame_index * TEST_VIDEO_GRAY_STEP );
    const ::cv::Scalar fill_color( gray_level, gray_level, gray_level );
    const ::cv::Mat frame( TEST_IMAGE_HEIGHT, TEST_IMAGE_WIDTH, CV_8UC3, fill_color );
    writer.write( frame );
    }

  writer.release();
  return full_path;
  }

} // namespace

TEST_CASE( "StillImageFrameSource : une frame puis fin de flux" )
{
  const ::std::string image_path = write_test_image( "still_source.jpg" );
  StillImageFrameSource source( image_path );

  const bool opened = source.is_opened();
  REQUIRE( true == opened );

  ::cv::Mat frame;
  const bool first_read = source.read( frame );
  const bool second_read = source.read( frame );

  CHECK( true == first_read );
  CHECK( TEST_IMAGE_WIDTH == frame.cols );
  CHECK( TEST_IMAGE_HEIGHT == frame.rows );
  CHECK( false == second_read );
}

TEST_CASE( "StillImageFrameSource : chemin invalide -> source non ouverte" )
{
  const ::std::string missing_path = test_temp_dir() + "/fichier_absent_line_detector.jpg";
  StillImageFrameSource source( missing_path );

  const bool opened = source.is_opened();

  ::cv::Mat frame;
  const bool read_ok = source.read( frame );

  CHECK( false == opened );
  CHECK( false == read_ok );
}

TEST_CASE( "CaptureFrameSource : lit toutes les frames d'un fichier video" )
{
  const ::std::string video_path = write_test_video( "capture_source.avi" );
  const ::std::unique_ptr< CaptureFrameSource > source = CaptureFrameSource::from_file( video_path );

  const bool opened = source->is_opened();
  REQUIRE( true == opened );

  int read_count = 0;
  ::cv::Mat frame;
  bool read_ok = source->read( frame );

  while ( read_ok )
    {
    ++read_count;
    CHECK( TEST_IMAGE_WIDTH == frame.cols );
    CHECK( TEST_IMAGE_HEIGHT == frame.rows );
    read_ok = source->read( frame );
    }

  CHECK( TEST_VIDEO_FRAME_COUNT == read_count );
}

TEST_CASE( "CaptureFrameSource : fichier absent -> source non ouverte" )
{
  const ::std::string missing_path = test_temp_dir() + "/video_absente_line_detector.avi";
  const ::std::unique_ptr< CaptureFrameSource > source = CaptureFrameSource::from_file( missing_path );

  const bool opened = source->is_opened();

  ::cv::Mat frame;
  const bool read_ok = source->read( frame );

  CHECK( false == opened );
  CHECK( false == read_ok );
}
