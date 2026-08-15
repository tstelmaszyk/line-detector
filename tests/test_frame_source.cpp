#include "doctest.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <string>

#include "StillImageFrameSource.h"
#include "test_support.h"

namespace
{

const int TEST_IMAGE_WIDTH = 64;    ///< Largeur des images de test.
const int TEST_IMAGE_HEIGHT = 48;   ///< Hauteur des images de test.
const int TEST_GRAY_LEVEL = 120;    ///< Niveau de gris de remplissage.

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
