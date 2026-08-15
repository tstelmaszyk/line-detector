#pragma once

/// @file
/// @brief Source de frames adossée à cv::VideoCapture (fichier ou caméra).

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <memory>
#include <string>

#include "FrameSource.h"

/// @brief Source de frames lisant un fichier vidéo ou une caméra.
///
/// Une seule classe pour les deux cas : cv::VideoCapture les traite de la même
/// façon. La construction passe par les fabriques, qui disent laquelle des deux
/// natures est demandée.
class CaptureFrameSource : public FrameSource
  {
  public:
    /// @brief Fabrique une source lisant un fichier vidéo.
    /// @param p_video_path Chemin du fichier vidéo.
    /// @return Source construite (vérifier is_opened()).
    static ::std::unique_ptr< CaptureFrameSource > from_file( const ::std::string& p_video_path );

    /// @brief Fabrique une source lisant une caméra.
    /// @param p_camera_index Index de la caméra.
    /// @return Source construite (vérifier is_opened()).
    static ::std::unique_ptr< CaptureFrameSource > from_camera( int p_camera_index );

    /// @brief Indique si la capture est ouverte.
    /// @return true si la source est exploitable.
    bool is_opened() const;

    /// @brief Lit la frame suivante.
    /// @param p_frame Frame lue (BGR).
    /// @return true si une frame a été lue, false à la fin du flux.
    bool read( ::cv::Mat& p_frame ) override;

  private:
    /// @brief Construit une source vide (usage réservé aux fabriques).
    CaptureFrameSource();

    ::cv::VideoCapture m_capture;  ///< Capture OpenCV sous-jacente.
  };
