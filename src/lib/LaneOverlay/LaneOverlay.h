#pragma once

/// @file
/// @brief Dessin de la voie détectée et du HUD sur l'image d'origine.

#include <opencv2/core.hpp>

#include "ImageSink/ImageSink.h"
#include "LaneModel/LaneModel.h"
#include "PerspectiveView/PerspectiveView.h"

/// @brief Dessine la voie (polygone entre les deux polynômes en BEV), la ramène
/// en perspective image, la fusionne sur l'image d'origine et ajoute un HUD.
class LaneOverlay
  {
  public:
    /// @brief Construit l'overlay.
    /// @param p_perspective Transformations perspective (warp inverse).
    /// @param p_debug_sink  Destination des traces de debug.
    LaneOverlay( const PerspectiveView& p_perspective, ImageSink& p_debug_sink );

    /// @brief Rend la voie et le HUD.
    /// @param p_original_bgr Image d'origine (BGR).
    /// @param p_model        Modèle de voie (signal de pilotage).
    /// @param p_output       Image de sortie.
    void render( const ::cv::Mat& p_original_bgr,
                 const LaneModel& p_model,
                 ::cv::Mat& p_output ) const;

  private:
    const PerspectiveView& m_perspective;  ///< Transformations perspective.
    ImageSink& m_debug_sink;                ///< Destination des traces.
  };
