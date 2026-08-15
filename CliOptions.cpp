/// @file
/// @brief Implémentation de l'analyse des arguments de ligne de commande.

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <string>

#include "CliOptions.h"

namespace
{

const ::std::string FLAG_IMAGE = "--image";    ///< Flag du mode image fixe.
const ::std::string FLAG_VIDEO = "--video";    ///< Flag du mode fichier vidéo.
const ::std::string FLAG_CAMERA = "--camera";  ///< Flag du mode caméra.

const ::std::string ERROR_CONFLICTING_MODES =
  "Modes exclusifs : un seul parmi --image, --video, --camera";  ///< Deux modes demandés.
const ::std::string ERROR_MISSING_VALUE = "Valeur manquante apres : ";   ///< Flag sans valeur.
const ::std::string ERROR_UNKNOWN_FLAG = "Flag inconnu : ";              ///< Flag non reconnu.
const ::std::string ERROR_POSITIONAL = "Argument positionnel non supporte : ";  ///< argv nu.
const ::std::string ERROR_CAMERA_INDEX = "Index camera invalide : ";     ///< Index non numérique.

const int FIRST_ARGUMENT_INDEX = 1;  ///< Premier argument utile (argv[0] = nom du programme).
const char FLAG_PREFIX = '-';        ///< Préfixe d'un flag.
const int DECIMAL_BASE = 10;         ///< Base de conversion de l'index caméra.

/// @brief Indique si un argument a la forme d'un flag.
/// @param p_argument Argument à tester.
/// @return true si l'argument commence par un tiret.
bool looks_like_flag( const ::std::string& p_argument )
  {
  const bool is_empty = p_argument.empty();

  if ( is_empty )
    {
    return false;
    }

  const bool starts_with_dash = ( FLAG_PREFIX == p_argument[0] );
  return starts_with_dash;
  }

} // namespace

int parse_arguments( int p_argument_count, char** p_arguments, CliOptions& p_options )
{
  bool mode_already_set = false;
  int argument_index = FIRST_ARGUMENT_INDEX;

  while ( argument_index < p_argument_count )
    {
    const ::std::string argument = p_arguments[argument_index];

    // Un mode a deja ete choisi et un nouveau flag de mode apparait : conflit.
    const bool is_image_flag = ( FLAG_IMAGE == argument );
    const bool is_video_flag = ( FLAG_VIDEO == argument );
    const bool is_camera_flag = ( FLAG_CAMERA == argument );
    const bool is_mode_flag = ( is_image_flag || is_video_flag || is_camera_flag );

    if ( is_mode_flag && mode_already_set )
      {
      p_options.error_message = ERROR_CONFLICTING_MODES;
      return EXIT_FAILURE;
      }

    if ( is_image_flag || is_video_flag )
      {
      // Modes a valeur obligatoire : le chemin suit immediatement le flag.
      const int value_index = argument_index + 1;
      const bool value_is_missing = ( value_index >= p_argument_count );

      if ( value_is_missing )
        {
        p_options.error_message = ERROR_MISSING_VALUE + argument;
        return EXIT_FAILURE;
        }

      p_options.source_kind = is_image_flag ? SOURCE_KIND_IMAGE : SOURCE_KIND_VIDEO;
      p_options.input_path = p_arguments[value_index];
      mode_already_set = true;
      argument_index = value_index + 1;
      }
    else if ( is_camera_flag )
      {
      // Mode a valeur optionnelle : l'index suit le flag, ou vaut le defaut.
      p_options.source_kind = SOURCE_KIND_CAMERA;
      p_options.camera_index = DEFAULT_CAMERA_INDEX;
      mode_already_set = true;
      argument_index = argument_index + 1;

      const bool has_next_argument = ( argument_index < p_argument_count );

      if ( has_next_argument )
        {
        const ::std::string next_argument = p_arguments[argument_index];
        const bool next_is_flag = looks_like_flag( next_argument );

        if ( !next_is_flag )
          {
          errno = 0;
          char* conversion_end = nullptr;
          const long converted_index =
            ::std::strtol( next_argument.c_str(), &conversion_end, DECIMAL_BASE );
          const bool conversion_ok =
            ( ( nullptr != conversion_end ) && ( '\0' == *conversion_end ) && !next_argument.empty() );
          const bool has_range_error = ( ERANGE == errno );
          const bool exceeds_int_max = ( INT_MAX < converted_index );
          const bool exceeds_int_min = ( INT_MIN > converted_index );

          if ( !conversion_ok || has_range_error || exceeds_int_max || exceeds_int_min )
            {
            p_options.error_message = ERROR_CAMERA_INDEX + next_argument;
            return EXIT_FAILURE;
            }

          p_options.camera_index = static_cast< int >( converted_index );
          argument_index = argument_index + 1;
          }
        }
      }
    else
      {
      const bool is_flag = looks_like_flag( argument );
      p_options.error_message = is_flag ? ( ERROR_UNKNOWN_FLAG + argument )
                                        : ( ERROR_POSITIONAL + argument );
      return EXIT_FAILURE;
      }
    }

  return EXIT_SUCCESS;
}
