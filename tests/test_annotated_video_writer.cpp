#include "doctest.h"

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <string>

#include "AnnotatedVideoWriter.h"
#include "LaneModel.h"
#include "test_support.h"

namespace
{

const int VIDEO_TEST_WIDTH = 64;      ///< Largeur des frames ecrites.
const int VIDEO_TEST_HEIGHT = 48;     ///< Hauteur des frames ecrites.
const int VIDEO_TEST_FRAMES = 4;      ///< Nombre de frames ecrites.
const double VIDEO_TEST_FPS = 10.0;   ///< Cadence demandee.
const double VIDEO_TEST_MS = 5.0;     ///< Duree de traitement simulee.
const int VIDEO_TEST_GRAY = 90;       ///< Niveau de gris des frames.

} // namespace

TEST_CASE( "AnnotatedVideoWriter : ecrit une video relisible" )
{
  const ::std::string video_path = test_temp_dir() + "/annotated_writer.avi";
  const ::cv::Scalar fill_color( VIDEO_TEST_GRAY, VIDEO_TEST_GRAY, VIDEO_TEST_GRAY );
  const ::cv::Mat frame( VIDEO_TEST_HEIGHT, VIDEO_TEST_WIDTH, CV_8UC3, fill_color );
  LaneModel model;

    {
    AnnotatedVideoWriter writer( video_path, VIDEO_TEST_FPS );

    for ( int frame_index = 0; frame_index < VIDEO_TEST_FRAMES; ++frame_index )
      {
      writer.on_frame( frame_index, model, frame, VIDEO_TEST_MS, 0.0 );
      }

    const bool failed = writer.has_failed();
    CHECK( false == failed );
    }

  // Le writer est detruit : le fichier est ferme et relisible.
  ::cv::VideoCapture capture( video_path );
  const bool opened = capture.isOpened();
  REQUIRE( true == opened );

  int read_count = 0;
  ::cv::Mat read_frame;
  bool read_ok = capture.read( read_frame );

  while ( read_ok )
    {
    ++read_count;
    CHECK( VIDEO_TEST_WIDTH == read_frame.cols );
    CHECK( VIDEO_TEST_HEIGHT == read_frame.rows );
    read_ok = capture.read( read_frame );
    }

  CHECK( VIDEO_TEST_FRAMES == read_count );
}

TEST_CASE( "AnnotatedVideoWriter : chemin invalide -> echec signale" )
{
  const ::std::string invalid_path = test_temp_dir() + "/dossier_inexistant_line_detector/sortie.avi";
  const ::cv::Scalar fill_color( VIDEO_TEST_GRAY, VIDEO_TEST_GRAY, VIDEO_TEST_GRAY );
  const ::cv::Mat frame( VIDEO_TEST_HEIGHT, VIDEO_TEST_WIDTH, CV_8UC3, fill_color );
  LaneModel model;

  AnnotatedVideoWriter writer( invalid_path, VIDEO_TEST_FPS );
  writer.on_frame( 0, model, frame, VIDEO_TEST_MS, 0.0 );

  const bool failed = writer.has_failed();

  CHECK( true == failed );
}

TEST_CASE( "AnnotatedVideoWriter : apres un echec, plus aucune frame n'est ecrite" )
{
  const ::std::string video_path = test_temp_dir() + "/annotated_writer_apres_echec.avi";
  const ::cv::Scalar fill_color( VIDEO_TEST_GRAY, VIDEO_TEST_GRAY, VIDEO_TEST_GRAY );
  const ::cv::Mat valid_frame( VIDEO_TEST_HEIGHT, VIDEO_TEST_WIDTH, CV_8UC3, fill_color );
  const ::cv::Mat empty_frame;
  LaneModel model;

    {
    AnnotatedVideoWriter writer( video_path, VIDEO_TEST_FPS );

    // Une frame vide met le writer en echec...
    writer.on_frame( 0, model, empty_frame, VIDEO_TEST_MS, 0.0 );

    // ... et la frame valide suivante ne doit plus rien ecrire.
    writer.on_frame( 1, model, valid_frame, VIDEO_TEST_MS, 0.0 );

    const bool failed = writer.has_failed();
    CHECK( true == failed );
    }

  // Aucune frame ecrite : le fichier est absent ou illisible.
  ::cv::VideoCapture capture( video_path );
  const bool opened = capture.isOpened();

  int read_count = 0;

  if ( true == opened )
    {
    ::cv::Mat read_frame;
    bool read_ok = capture.read( read_frame );

    while ( read_ok )
      {
      ++read_count;
      read_ok = capture.read( read_frame );
      }
    }

  CHECK( 0 == read_count );
}
