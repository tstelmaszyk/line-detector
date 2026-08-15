/// @file
/// @brief Point d'entrée : assemble source, détecteur et observateurs, puis boucle.

#include <opencv2/core.hpp>

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "AnnotatedVideoWriter.h"
#include "CaptureFrameSource.h"
#include "CliOptions.h"
#include "DetectLines.h"
#include "DiskImageSink.h"
#include "FrameObserver.h"
#include "FrameSource.h"
#include "ImageSink.h"
#include "LaneConfig.h"
#include "LaneModelLogger.h"
#include "NullImageSink.h"
#include "PipelineRunner.h"
#include "ResultImageWriter.h"
#include "RunStats.h"
#include "StillImageFrameSource.h"
#include "VideoCaracteristics.h"

namespace
{

const char* const DEFAULT_OUTPUT_DIR = "out";                ///< Dossier de sortie par défaut.
const char* const OUTPUT_DIR_ENV_VAR = "LINE_DETECTOR_OUT";  ///< Variable d'env du dossier de sortie.
const char* const DEBUG_ENV_VAR = "LINE_DETECTOR_DEBUG";     ///< Variable d'env d'activation du debug.
const ::std::string OUTPUT_IMAGE_NAME = "output.jpg";        ///< Nom du fichier image résultat.
const ::std::string OUTPUT_VIDEO_NAME = "output.avi";        ///< Nom du fichier vidéo résultat.
const ::std::string PATH_SEPARATOR = "/";                    ///< Séparateur de chemin.
const double DEFAULT_LANE_WIDTH_RATIO = 0.35;                ///< Largeur de voie par défaut (fraction de W).
const double OUTPUT_VIDEO_FPS = 30.0;                        ///< Cadence déclarée de la vidéo de sortie.
const double MILLISECONDS_PER_SECOND = 1000.0;               ///< Conversion ms -> s.

const ::std::string USAGE_MESSAGE =
  "Usage : line_detector [--image <chemin> | --video <chemin> | --camera [index]]";  ///< Aide.

/// @brief Drapeau d'arrêt levé par le handler SIGINT.
::std::atomic< bool > g_stop_requested( false );

/// @brief Handler SIGINT : demande l'arrêt de la boucle (fermeture propre des sorties).
/// @param p_signal_number Numéro du signal reçu.
void handle_interrupt( int p_signal_number )
  {
  (void) p_signal_number;
  g_stop_requested.store( true );
  }

/// @brief Construit la source de frames correspondant aux options.
/// @param p_options Options de la ligne de commande.
/// @return Source construite, ou nullptr si elle n'a pas pu être ouverte.
::std::unique_ptr< FrameSource > make_frame_source( const CliOptions& p_options )
  {
  if ( SOURCE_KIND_CAMERA == p_options.source_kind )
    {
    ::std::unique_ptr< CaptureFrameSource > camera_source =
      CaptureFrameSource::from_camera( p_options.camera_index );
    const bool opened = camera_source->is_opened();

    if ( !opened )
      {
      return nullptr;
      }

    return camera_source;
    }

  if ( SOURCE_KIND_VIDEO == p_options.source_kind )
    {
    ::std::unique_ptr< CaptureFrameSource > video_source =
      CaptureFrameSource::from_file( p_options.input_path );
    const bool opened = video_source->is_opened();

    if ( !opened )
      {
      return nullptr;
      }

    return video_source;
    }

  ::std::unique_ptr< StillImageFrameSource > image_source =
    ::std::make_unique< StillImageFrameSource >( p_options.input_path );
  const bool opened = image_source->is_opened();

  if ( !opened )
    {
    return nullptr;
    }

  return image_source;
  }

} // namespace

int main( int argc, char** argv )
{
  // 1. Analyse des arguments.
  CliOptions options;
  const int parse_status = parse_arguments( argc, argv, options );

  if ( EXIT_SUCCESS != parse_status )
    {
    ::std::cout << options.error_message << ::std::endl;
    ::std::cout << USAGE_MESSAGE << ::std::endl;
    return EXIT_FAILURE;
    }

  // 2. Ouverture de la source de frames.
  ::std::unique_ptr< FrameSource > frame_source = make_frame_source( options );

  if ( nullptr == frame_source )
    {
    ::std::cout << "Impossible d'ouvrir la source demandee." << ::std::endl;
    return EXIT_FAILURE;
    }

  // 3. Premiere frame : elle definit la geometrie de tout le pipeline.
  ::cv::Mat first_frame;
  const bool first_read_ok = frame_source->read( first_frame );

  if ( !first_read_ok )
    {
    ::std::cout << "Aucune frame lisible dans la source." << ::std::endl;
    return EXIT_FAILURE;
    }

  // 4. Dossier de sortie et traces de debug.
  const char* output_dir_env = ::std::getenv( OUTPUT_DIR_ENV_VAR );
  const ::std::string output_dir = ( nullptr != output_dir_env ) ? output_dir_env : DEFAULT_OUTPUT_DIR;

  const char* debug_env = ::std::getenv( DEBUG_ENV_VAR );
  const bool debug_enabled = ( nullptr != debug_env ) && ( '\0' != debug_env[0] );

  ::std::unique_ptr< ImageSink > debug_sink;

  if ( debug_enabled )
    {
    debug_sink = ::std::make_unique< DiskImageSink >( output_dir );
    }
  else
    {
    debug_sink = ::std::make_unique< NullImageSink >();
    }

  // 5. Detecteur.
  VideoCaracteristics video_properties( first_frame );

  LaneConfig config;
  // Largeur de voie par defaut (pixels BEV) pour reconstruire un cote manquant.
  config.default_lane_width_px =
    static_cast< double >( video_properties.width_pixel ) * DEFAULT_LANE_WIDTH_RATIO;

  const DetectLines detector( video_properties, config, *debug_sink );

  // 6. Observateurs : log toujours, puis image (mode image) ou video (mode flux).
  LaneModelLogger logger( ::std::cout );
  DiskImageSink result_sink( output_dir );
  ResultImageWriter image_writer( result_sink, OUTPUT_IMAGE_NAME );

  const ::std::string video_path = output_dir + PATH_SEPARATOR + OUTPUT_VIDEO_NAME;
  AnnotatedVideoWriter video_writer( video_path, OUTPUT_VIDEO_FPS );

  const bool is_still_image = ( SOURCE_KIND_IMAGE == options.source_kind );

  ::std::vector< FrameObserver* > observers;
  observers.push_back( &logger );

  if ( is_still_image )
    {
    observers.push_back( &image_writer );
    }
  else
    {
    observers.push_back( &video_writer );
    }

  // 7. Arret propre : sans cela, Ctrl-C laisse la video de sortie inexploitable.
  ::std::signal( SIGINT, handle_interrupt );

  // 8. Boucle.
  PipelineRunner runner( *frame_source, detector, observers, g_stop_requested );
  RunStats stats;
  const int run_status = runner.run( first_frame, stats );

  if ( EXIT_SUCCESS != run_status )
    {
    ::std::cout << "Aucune frame traitee." << ::std::endl;
    return EXIT_FAILURE;
    }

  // 9. Verification des sorties.
  const bool image_failed = ( is_still_image && image_writer.has_failed() );
  const bool video_failed = ( !is_still_image && video_writer.has_failed() );

  if ( image_failed )
    {
    ::std::cout << "Impossible d'ecrire l'image de sortie : "
                << output_dir << PATH_SEPARATOR << OUTPUT_IMAGE_NAME << ::std::endl;
    return EXIT_FAILURE;
    }

  if ( video_failed )
    {
    ::std::cout << "Impossible d'ecrire la video de sortie : " << video_path << ::std::endl;
    return EXIT_FAILURE;
    }

  // 10. Resume.
  const double average_ms = stats.total_ms / static_cast< double >( stats.frame_count );
  const double frames_per_second = MILLISECONDS_PER_SECOND / average_ms;

  ::std::cout << "Frames : " << stats.frame_count
              << " | detectees : " << stats.detected_count
              << " | reconstruites : " << stats.reconstructed_count
              << " | moyenne : " << average_ms << " ms/frame"
              << " (" << frames_per_second << " FPS)" << ::std::endl;

  const ::std::string result_name = is_still_image ? OUTPUT_IMAGE_NAME : OUTPUT_VIDEO_NAME;
  ::std::cout << "Resultat ecrit dans : " << output_dir << PATH_SEPARATOR << result_name << ::std::endl;

  return EXIT_SUCCESS;
}
