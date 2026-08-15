#pragma once

/// @file
/// @brief Utilitaires partages par les tests.

#include <cstdlib>
#include <string>

#ifndef LINE_DETECTOR_TEST_TMP_DEFAULT
#define LINE_DETECTOR_TEST_TMP_DEFAULT "."
#endif

/// @brief Variable d'environnement du dossier temporaire des tests.
static const ::std::string TEST_TMP_ENV_VAR = "LINE_DETECTOR_TEST_TMP";

/// @brief Variable d'environnement standard du dossier temporaire.
static const ::std::string SYSTEM_TMP_ENV_VAR = "TMPDIR";

/// @brief Dossier de repli si aucune variable d'environnement n'est definie.
/// Valeur injectee par CMake (dossier de build) via LINE_DETECTOR_TEST_TMP_DEFAULT ;
/// "." seulement si le define est absent (hors build CMake normal).
static const ::std::string FALLBACK_TMP_DIR = LINE_DETECTOR_TEST_TMP_DEFAULT;

/// @brief Donne le dossier ou les tests ecrivent leurs fichiers temporaires.
/// @return Chemin de dossier, sans separateur final.
inline ::std::string test_temp_dir()
{
  const char* configured_dir = ::std::getenv( TEST_TMP_ENV_VAR.c_str() );

  if ( nullptr != configured_dir )
    {
    const ::std::string result = configured_dir;
    return result;
    }

  const char* system_dir = ::std::getenv( SYSTEM_TMP_ENV_VAR.c_str() );

  if ( nullptr != system_dir )
    {
    const ::std::string result = system_dir;
    return result;
    }

  return FALLBACK_TMP_DIR;
}
