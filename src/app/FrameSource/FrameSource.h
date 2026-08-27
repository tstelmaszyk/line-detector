#pragma once

/// @file
/// @brief Interface de source de frames (fichier vidéo, caméra, image fixe).

#include <opencv2/core.hpp>

/// @brief Source de frames consommée par la boucle applicative.
///
/// La bibliothèque de détection ne connaît pas ce type : la provenance des
/// pixels est une décision de l'application.
class FrameSource
  {
  public:
    /// @brief Destructeur virtuel par défaut.
    virtual ~FrameSource() = default;

    /// @brief Lit la frame suivante.
    /// @param p_frame Frame lue (BGR) ; non modifiée si la lecture échoue.
    /// @return true si une frame a été lue, false à la fin du flux.
    virtual bool read( ::cv::Mat& p_frame ) = 0;
  };
