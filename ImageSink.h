#pragma once

/// @file
/// @brief Interface de destination d'écriture d'image (résultat ou debug).

#include <opencv2/core.hpp>

#include <string>

/// @brief Destination d'écriture d'une image (résultat final ou trace de debug).
///
/// `p_name` est le nom de fichier complet (ex. "output.jpg"). Aucun préfixe ni
/// extension n'est imposé : le même sink sert au résultat comme aux traces.
class ImageSink
  {
  public:
    /// @brief Destructeur virtuel par défaut.
    virtual ~ImageSink() = default;

    /// @brief Écrit une image.
    /// @param p_name  Nom de fichier complet.
    /// @param p_frame Image à écrire.
    /// @return true si l'image a été écrite.
    virtual bool save( const ::std::string& p_name, const ::cv::Mat& p_frame ) = 0;
  };
