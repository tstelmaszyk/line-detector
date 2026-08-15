#pragma once

/// @file
/// @brief Utilitaires partages par les tests.

#include <cstdlib>
#include <string>

/// @brief Variable d'environnement du dossier temporaire des tests.
static const ::std::string TEST_TMP_ENV_VAR = "LINE_DETECTOR_TEST_TMP";

/// @brief Variable d'environnement standard du dossier temporaire.
static const ::std::string SYSTEM_TMP_ENV_VAR = "TMPDIR";

/// @brief Dossier de repli si aucune variable d'environnement n'est definie.
static const ::std::string FALLBACK_TMP_DIR = ".";

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
