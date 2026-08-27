#pragma once

/// @file
/// @brief Options de ligne de commande et analyse des arguments.

#include <string>

/// @brief Nature de la source de frames demandée.
enum e_source_kind
  {
  SOURCE_KIND_IMAGE = 0,  ///< Image fixe.
  SOURCE_KIND_VIDEO,      ///< Fichier vidéo.
  SOURCE_KIND_CAMERA,     ///< Caméra.
  SOURCE_KIND_COUNT       ///< Nombre d'éléments.
  };

/// @brief Message d'aide affiché sur stderr en cas d'arguments invalides.
extern const ::std::string USAGE_MESSAGE;

/// @brief Chemin d'image utilisé quand aucun argument n'est fourni.
static const ::std::string DEFAULT_IMAGE_PATH = "img_piste/img2.jpg";

/// @brief Index de caméra utilisé quand --camera est donné sans valeur.
static const int DEFAULT_CAMERA_INDEX = 0;

/// @brief Options issues de la ligne de commande.
struct CliOptions
  {
  e_source_kind source_kind = SOURCE_KIND_IMAGE;  ///< Mode demandé.
  ::std::string input_path = DEFAULT_IMAGE_PATH;  ///< Chemin (modes image et vidéo).
  int camera_index = DEFAULT_CAMERA_INDEX;        ///< Index caméra (mode caméra).
  bool record = false;                            ///< true si --record est passé : écrit le résultat sur disque.
  ::std::string error_message;                    ///< Vide si les arguments sont valides.
  };

/// @brief Analyse les arguments de la ligne de commande.
///
/// Modes mutuellement exclusifs : --image <chemin>, --video <chemin>,
/// --camera [index]. Sans argument, le mode image par défaut est utilisé.
/// @param p_argument_count Nombre d'arguments (argv[0] inclus).
/// @param p_arguments      Tableau d'arguments.
/// @param p_options        Options remplies en sortie (error_message en cas d'échec).
/// @return EXIT_SUCCESS si les arguments sont valides, EXIT_FAILURE sinon.
int parse_arguments( int p_argument_count, char** p_arguments, CliOptions& p_options );
