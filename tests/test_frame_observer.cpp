#include "doctest.h"

#include <opencv2/core.hpp>

#include <sstream>
#include <string>

#include "ImageSink.h"
#include "LaneModel.h"
#include "LaneModelLogger.h"
#include "ResultImageWriter.h"

namespace
{

const int OBSERVER_TEST_WIDTH = 32;     ///< Largeur des frames de test.
const int OBSERVER_TEST_HEIGHT = 24;    ///< Hauteur des frames de test.
const double TEST_ELAPSED_MS = 12.5;    ///< Duree de traitement simulee.

/// @brief Sink de test : memorise les appels au lieu d'ecrire sur disque.
class RecordingImageSink : public ImageSink
  {
  public:
    /// @brief Construit le sink avec le resultat a renvoyer.
    /// @param p_save_result Valeur rendue par save().
    explicit RecordingImageSink( bool p_save_result )
      : m_save_result( p_save_result ),
        m_save_count( 0 ),
        m_last_name()
      {
      }

    /// @brief Memorise l'appel.
    bool save( const ::std::string& p_name, const ::cv::Mat& p_frame ) override
      {
      (void) p_frame;
      ++m_save_count;
      m_last_name = p_name;
      return m_save_result;
      }

    /// @brief Nombre d'appels a save().
    int save_count() const
      {
      return m_save_count;
      }

    /// @brief Dernier nom de fichier recu.
    const ::std::string& last_name() const
      {
      return m_last_name;
      }

  private:
    bool m_save_result;         ///< Valeur rendue par save().
    int m_save_count;           ///< Nombre d'appels.
    ::std::string m_last_name;  ///< Dernier nom recu.
  };

/// @brief Construit un LaneModel de test.
LaneModel make_test_model( double p_normalized_offset )
  {
  LaneModel model;
  model.lane_detected = true;
  model.reconstructed = false;
  model.normalized_offset = p_normalized_offset;
  model.lateral_offset_px = 42.0;
  model.curvature_radius_px = 1500.0;
  return model;
  }

} // namespace

TEST_CASE( "LaneModelLogger : en-tete puis une ligne par frame" )
{
  ::std::ostringstream output;
  LaneModelLogger logger( output );
  const ::cv::Mat frame( OBSERVER_TEST_HEIGHT, OBSERVER_TEST_WIDTH, CV_8UC3, ::cv::Scalar( 0, 0, 0 ) );

  const LaneModel first_model = make_test_model( -0.25 );
  const LaneModel second_model = make_test_model( 0.10 );
  logger.on_frame( 0, first_model, frame, TEST_ELAPSED_MS );
  logger.on_frame( 1, second_model, frame, TEST_ELAPSED_MS );

  const ::std::string text = output.str();

  // Une ligne d'en-tete + deux lignes de donnees.
  int line_count = 0;

  for ( const char character : text )
    {
    if ( '\n' == character )
      {
      ++line_count;
      }
    }

  CHECK( 3 == line_count );
  CHECK( ::std::string::npos != text.find( "frame_index" ) );
  CHECK( ::std::string::npos != text.find( "normalized_offset" ) );
  CHECK( ::std::string::npos != text.find( "-0.25" ) );
}

TEST_CASE( "ResultImageWriter : ecrit la frame annotee via l'ImageSink" )
{
  RecordingImageSink sink( true );
  ResultImageWriter writer( sink, "output.jpg" );
  const ::cv::Mat frame( OBSERVER_TEST_HEIGHT, OBSERVER_TEST_WIDTH, CV_8UC3, ::cv::Scalar( 0, 0, 0 ) );
  const LaneModel model = make_test_model( 0.0 );

  writer.on_frame( 0, model, frame, TEST_ELAPSED_MS );

  const bool failed = writer.has_failed();

  CHECK( 1 == sink.save_count() );
  CHECK( ::std::string( "output.jpg" ) == sink.last_name() );
  CHECK( false == failed );
}

TEST_CASE( "ResultImageWriter : echec d'ecriture signale par has_failed" )
{
  RecordingImageSink sink( false );
  ResultImageWriter writer( sink, "output.jpg" );
  const ::cv::Mat frame( OBSERVER_TEST_HEIGHT, OBSERVER_TEST_WIDTH, CV_8UC3, ::cv::Scalar( 0, 0, 0 ) );
  const LaneModel model = make_test_model( 0.0 );

  writer.on_frame( 0, model, frame, TEST_ELAPSED_MS );

  const bool failed = writer.has_failed();

  CHECK( true == failed );
}
