/// @file
/// @brief Point d'entrée : assemble source, détecteur et observateurs, puis boucle.

#include <opencv2/core.hpp>

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <system_error>
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
  "Usage : line_detector [--image <chemin> | --video <chemin> | --camera [index]] [--record]";  ///< Aide.

const ::std::string MESSAGE_UNABLE_TO_OPEN_SOURCE =
  "Impossible d'ouvrir la source demandee.";                             ///< Source non ouverte.
const ::std::string MESSAGE_NO_READABLE_FRAME =
  "Aucune frame lisible dans la source.";                                ///< Source vide.
const ::std::string MESSAGE_NO_FRAME_PROCESSED = "Aucune frame traitee.";  ///< Boucle vide.
const ::std::string MESSAGE_IMAGE_WRITE_FAILED_PREFIX =
  "Impossible d'ecrire l'image de sortie : ";                            ///< Echec ecriture image.
const ::std::string MESSAGE_VIDEO_WRITE_FAILED_PREFIX =
  "Impossible d'ecrire la video de sortie : ";                           ///< Echec ecriture video.
const ::std::string MESSAGE_OUTPUT_DIR_FAILED_PREFIX =
  "Impossible de creer ou d'utiliser le dossier de sortie : ";           ///< Echec dossier de sortie.
const ::std::string MESSAGE_SUMMARY_FRAMES_PREFIX = "Frames : ";                    ///< Resume : frames.
const ::std::string MESSAGE_SUMMARY_DETECTED_PREFIX = " | detectees : ";            ///< Resume : detections.
const ::std::string MESSAGE_SUMMARY_RECONSTRUCTED_PREFIX = " | reconstruites : ";   ///< Resume : reconstructions.
const ::std::string MESSAGE_SUMMARY_AVERAGE_PREFIX = " | moyenne : ";               ///< Resume : moyenne ms.
const ::std::string MESSAGE_SUMMARY_MS_PER_FRAME_SUFFIX = " ms/frame";              ///< Resume : suffixe ms/frame.
const ::std::string MESSAGE_SUMMARY_RENDER_PREFIX = " (dont ";                      ///< Resume : part du rendu.
const ::std::string MESSAGE_SUMMARY_RENDER_SUFFIX = " ms de rendu)";                ///< Resume : fin rendu.
const ::std::string MESSAGE_SUMMARY_FPS_PREFIX = " (";                              ///< Resume : ouverture FPS.
const ::std::string MESSAGE_SUMMARY_FPS_SUFFIX = " FPS)";                           ///< Resume : fermeture FPS.
const ::std::string MESSAGE_RESULT_WRITTEN_PREFIX = "Resultat ecrit dans : ";       ///< Resume : chemin resultat.

/// @brief Drapeau d'arrêt levé par le handler SIGINT.
::std::atomic< bool > g_stop_requested( false );

/// @brief Handler SIGINT : demande l'arrêt de la boucle (fermeture propre des sorties).
///
/// Restaure d'abord l'action par défaut : un second Ctrl-C doit toujours pouvoir
/// terminer le processus, meme si VideoCapture::read est bloque sur une camera
/// debranchee et ne revient jamais tester g_stop_requested.
/// @param p_signal_number Numéro du signal reçu.
void handle_interrupt( int p_signal_number )
  {
  ::std::signal( SIGINT, SIG_DFL );
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

/// @brief Point d'entrée : assemble la source, le détecteur et les observateurs,
/// puis délègue la boucle au PipelineRunner.
/// @param argc Nombre d'arguments.
/// @param argv Arguments de la ligne de commande.
/// @return EXIT_SUCCESS si le traitement s'est terminé normalement, EXIT_FAILURE sinon.
int main( int argc, char** argv )
{
  // 1. Analyse des arguments.
  CliOptions options;
  const int parse_status = parse_arguments( argc, argv, options );

  if ( EXIT_SUCCESS != parse_status )
    {
    ::std::cerr << options.error_message << ::std::endl;
    ::std::cerr << USAGE_MESSAGE << ::std::endl;
    return EXIT_FAILURE;
    }

  // 2. Ouverture de la source de frames.
  ::std::unique_ptr< FrameSource > frame_source = make_frame_source( options );

  if ( nullptr == frame_source )
    {
    ::std::cerr << MESSAGE_UNABLE_TO_OPEN_SOURCE << ::std::endl;
    return EXIT_FAILURE;
    }

  // 3. Premiere frame : elle definit la geometrie de tout le pipeline.
  ::cv::Mat first_frame;
  const bool first_read_ok = frame_source->read( first_frame );

  if ( !first_read_ok )
    {
    ::std::cerr << MESSAGE_NO_READABLE_FRAME << ::std::endl;
    return EXIT_FAILURE;
    }

  // 4. Dossier de sortie et traces de debug.
  const char* output_dir_env = ::std::getenv( OUTPUT_DIR_ENV_VAR );
  const ::std::string output_dir = ( nullptr != output_dir_env ) ? output_dir_env : DEFAULT_OUTPUT_DIR;

  const char* debug_env = ::std::getenv( DEBUG_ENV_VAR );
  const bool debug_enabled = ( nullptr != debug_env ) && ( '\0' != debug_env[0] );

  // Le dossier de sortie n'est necessaire que si quelque chose sera ecrit :
  // le resultat avec --record, ou les traces avec LINE_DETECTOR_DEBUG. Sans
  // cela, le programme n'ecrit aucun fichier et ne doit pas echouer sur un
  // dossier de sortie inutilisable (camera en tete, rootfs eventuellement
  // en lecture seule).
  const bool needs_output_dir = ( options.record || debug_enabled );

  if ( needs_output_dir )
    {
    // cv::imwrite ne cree pas le dossier de sortie : on le cree ici, avant tout sink.
    // Version non-levante : un dossier deja present n'est pas une erreur (create_directories
    // rend false sans lever), mais des droits insuffisants ou un fichier deja present a ce
    // chemin ne doivent pas terminer le process par une exception non rattrapee.
    ::std::error_code create_directory_error;
    ::std::filesystem::create_directories( output_dir, create_directory_error );

    const bool output_dir_usable = ::std::filesystem::is_directory( output_dir, create_directory_error );

    if ( !output_dir_usable )
      {
      ::std::cerr << MESSAGE_OUTPUT_DIR_FAILED_PREFIX << output_dir << ::std::endl;
      return EXIT_FAILURE;
      }
    }

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

  // 6. Observateurs : le log CSV est toujours present ; les ecritures ne le sont
  // que si --record est passe. Sans writer, aucun observateur ne reclame l'image
  // annotee et le rendu n'est jamais execute.
  LaneModelLogger logger( ::std::cout );

  ::std::vector< FrameObserver* > observers;
  observers.push_back( &logger );

  const bool is_still_image = ( SOURCE_KIND_IMAGE == options.source_kind );
  const ::std::string video_path = output_dir + PATH_SEPARATOR + OUTPUT_VIDEO_NAME;

  ::std::unique_ptr< DiskImageSink > result_sink;
  ::std::unique_ptr< ResultImageWriter > image_writer;
  ::std::unique_ptr< AnnotatedVideoWriter > video_writer;

  if ( options.record )
    {
    if ( is_still_image )
      {
      result_sink = ::std::make_unique< DiskImageSink >( output_dir );
      image_writer = ::std::make_unique< ResultImageWriter >( *result_sink, OUTPUT_IMAGE_NAME );
      observers.push_back( image_writer.get() );
      }
    else
      {
      video_writer = ::std::make_unique< AnnotatedVideoWriter >( video_path, OUTPUT_VIDEO_FPS );
      observers.push_back( video_writer.get() );
      }
    }

  // 7. Arret propre : sans cela, Ctrl-C laisse la video de sortie inexploitable.
  ::std::signal( SIGINT, handle_interrupt );

  // 8. Boucle.
  PipelineRunner runner( *frame_source, detector, observers, g_stop_requested );
  RunStats stats;
  const int run_status = runner.run( first_frame, stats );

  if ( EXIT_SUCCESS != run_status )
    {
    ::std::cerr << MESSAGE_NO_FRAME_PROCESSED << ::std::endl;
    return EXIT_FAILURE;
    }

  // 9. Verification des sorties.
  const bool image_failed = ( nullptr != image_writer ) && image_writer->has_failed();
  const bool video_failed = ( nullptr != video_writer ) && video_writer->has_failed();

  if ( image_failed )
    {
    ::std::cerr << MESSAGE_IMAGE_WRITE_FAILED_PREFIX
                << output_dir << PATH_SEPARATOR << OUTPUT_IMAGE_NAME << ::std::endl;
    return EXIT_FAILURE;
    }

  if ( video_failed )
    {
    ::std::cerr << MESSAGE_VIDEO_WRITE_FAILED_PREFIX << video_path << ::std::endl;
    return EXIT_FAILURE;
    }

  // 10. Resume.
  const double total_ms = stats.compute_ms + stats.render_ms;
  const double average_ms = total_ms / static_cast< double >( stats.frame_count );
  const double average_render_ms = stats.render_ms / static_cast< double >( stats.frame_count );
  const double frames_per_second = MILLISECONDS_PER_SECOND / average_ms;

  ::std::cerr << MESSAGE_SUMMARY_FRAMES_PREFIX << stats.frame_count
              << MESSAGE_SUMMARY_DETECTED_PREFIX << stats.detected_count
              << MESSAGE_SUMMARY_RECONSTRUCTED_PREFIX << stats.reconstructed_count
              << MESSAGE_SUMMARY_AVERAGE_PREFIX << average_ms << MESSAGE_SUMMARY_MS_PER_FRAME_SUFFIX
              << MESSAGE_SUMMARY_RENDER_PREFIX << average_render_ms << MESSAGE_SUMMARY_RENDER_SUFFIX
              << MESSAGE_SUMMARY_FPS_PREFIX << frames_per_second
              << MESSAGE_SUMMARY_FPS_SUFFIX << ::std::endl;

  if ( options.record )
    {
    const ::std::string result_name = is_still_image ? OUTPUT_IMAGE_NAME : OUTPUT_VIDEO_NAME;
    ::std::cerr << MESSAGE_RESULT_WRITTEN_PREFIX << output_dir << PATH_SEPARATOR
                << result_name << ::std::endl;
    }

  return EXIT_SUCCESS;
}
