#pragma once

/// @file
/// @brief Sink disque : écrit l'image via cv::imwrite dans un dossier.

#include <opencv2/core.hpp>

#include <string>

#include "ImageSink/ImageSink.h"

/// @brief Sink disque : écrit l'image dans un dossier via cv::imwrite.
class DiskImageSink : public ImageSink
  {
  public:
    /// @brief Construit le sink pour un dossier de sortie.
    /// @param p_output_dir Dossier de sortie (doit exister).
    explicit DiskImageSink( ::std::string p_output_dir );

    /// @brief Écrit l'image dans le dossier de sortie.
    /// @param p_name  Nom de fichier complet.
    /// @param p_frame Image à écrire.
    /// @return true si l'écriture a réussi.
    bool save( const ::std::string& p_name, const ::cv::Mat& p_frame ) override;

  private:
    ::std::string m_output_dir;  ///< Dossier de sortie.
  };
