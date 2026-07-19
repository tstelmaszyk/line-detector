/// @file
/// @brief Point d'entrée : traite une image et écrit le résultat annoté.

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "DetectLines.h"
#include "DiskImageSink.h"
#include "ImageSink.h"
#include "LaneConfig.h"
#include "LaneModel.h"
#include "NullImageSink.h"
#include "VideoCaracteristics.h"

namespace
{

const char* const DEFAULT_INPUT_PATH = "img_piste/img2.jpg";  ///< Image d'entrée par défaut.
const char* const DEFAULT_OUTPUT_DIR = "out";                 ///< Dossier de sortie par défaut.
const char* const OUTPUT_DIR_ENV_VAR = "LINE_DETECTOR_OUT";   ///< Variable d'env du dossier de sortie.
const char* const DEBUG_ENV_VAR = "LINE_DETECTOR_DEBUG";      ///< Variable d'env d'activation du debug.
const ::std::string OUTPUT_FILE_NAME = "output.jpg";          ///< Nom du fichier résultat.
const double DEFAULT_LANE_WIDTH_RATIO = 0.35;                 ///< Largeur de voie par défaut (fraction de W).
const int MINIMUM_ARGC_WITH_PATH = 2;                         ///< argc minimal si un chemin est fourni.
const int INPUT_PATH_ARG_INDEX = 1;                           ///< Indice de l'argument chemin d'entrée.

} // namespace

int main( int argc, char** argv )
{
  // Chemin d'entrée : argv[1] si fourni, sinon défaut.
  const bool has_input_argument = ( argc >= MINIMUM_ARGC_WITH_PATH );
  const ::std::string input_path = has_input_argument ? argv[INPUT_PATH_ARG_INDEX] : DEFAULT_INPUT_PATH;

  // Dossier de sortie : variable d'environnement si définie, sinon défaut.
  const char* output_dir_env = ::std::getenv( OUTPUT_DIR_ENV_VAR );
  const ::std::string output_dir = ( nullptr != output_dir_env ) ? output_dir_env : DEFAULT_OUTPUT_DIR;

  ::cv::Mat image = ::cv::imread( input_path, ::cv::IMREAD_COLOR );

  if ( !image.data )
    {
    ::std::cout << "Impossible de lire l'image : " << input_path << ::std::endl;
    return EXIT_FAILURE;
    }

  DiskImageSink result_sink( output_dir );

  // Traces de debug : disque si LINE_DETECTOR_DEBUG non vide, sinon no-op.
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

  VideoCaracteristics video_properties( image );

  LaneConfig config;
  // Largeur de voie par défaut (pixels BEV) pour reconstruire un côté manquant.
  config.default_lane_width_px =
    static_cast< double >( video_properties.width_pixel ) * DEFAULT_LANE_WIDTH_RATIO;

  ::cv::Mat image_out;
  DetectLines detector( video_properties, config, *debug_sink );
  const LaneModel model = detector.draw_lines( image, image_out );

  ::std::cout << "Voie detectee : " << ( model.lane_detected ? "oui" : "non" )
              << " | offset normalise : " << model.normalized_offset
              << " | rayon : " << model.curvature_radius_px << " px"
              << " | reconstruit : " << ( model.reconstructed ? "oui" : "non" ) << ::std::endl;

  const bool saved = result_sink.save( OUTPUT_FILE_NAME, image_out );

  if ( !saved )
    {
    ::std::cout << "Impossible d'ecrire l'image de sortie : "
                << output_dir << "/" << OUTPUT_FILE_NAME << ::std::endl;
    return EXIT_FAILURE;
    }

  ::std::cout << "Image traitee ecrite dans : " << output_dir << "/" << OUTPUT_FILE_NAME << ::std::endl;
  return EXIT_SUCCESS;
}
