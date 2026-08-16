#pragma once

/// @file
/// @brief Source de frames : une image fixe, rendue une seule fois.

#include <opencv2/core.hpp>

#include <string>

#include "FrameSource.h"

/// @brief Source d'une image fixe : rend la frame une fois, puis la fin de flux.
///
/// Fait du mode image fixe un cas dégénéré du mode vidéo : un seul chemin de
/// code dans la boucle applicative.
class StillImageFrameSource : public FrameSource
  {
  public:
    /// @brief Charge l'image depuis le disque.
    /// @param p_image_path Chemin de l'image à charger.
    explicit StillImageFrameSource( const ::std::string& p_image_path );

    /// @brief Indique si l'image a pu être chargée.
    /// @return true si l'image est exploitable.
    bool is_opened() const;

    /// @brief Rend l'image au premier appel, la fin de flux ensuite.
    /// @param p_frame Frame lue (BGR).
    /// @return true au premier appel si l'image est chargée, false sinon.
    bool read( ::cv::Mat& p_frame ) override;

  private:
    ::cv::Mat m_image;           ///< Image chargée au démarrage.
    bool m_already_delivered;    ///< true une fois la frame rendue.
  };
