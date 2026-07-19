#pragma once

/// @file
/// @brief Orchestration de la chaîne de détection de voie.

#include <opencv2/core.hpp>

#include "ImageSink.h"
#include "LaneConfig.h"
#include "LaneMask.h"
#include "LaneModel.h"
#include "LaneOverlay.h"
#include "PerspectiveView.h"
#include "SlidingWindowSearch.h"
#include "VideoCaracteristics.h"

/// @brief Orchestre : masque -> BEV -> fenêtres glissantes -> fit polynomial ->
/// géométrie -> overlay. Renvoie le LaneModel et dessine le résultat.
class DetectLines
  {
  public:
    /// @brief Construit le détecteur.
    /// @param p_video      Caractéristiques image.
    /// @param p_config     Configuration du pipeline.
    /// @param p_debug_sink Destination des traces de debug.
    DetectLines( const VideoCaracteristics& p_video,
                 const LaneConfig& p_config,
                 ImageSink& p_debug_sink );

    /// @brief Exécute le pipeline complet.
    /// @param p_frame_to_compute Image d'entrée BGR.
    /// @param p_frame_with_lines Image de sortie annotée.
    /// @return Modèle de voie (signal de pilotage).
    LaneModel draw_lines( const ::cv::Mat& p_frame_to_compute, ::cv::Mat& p_frame_with_lines ) const;

  private:
    const VideoCaracteristics m_video_properties;  ///< Caractéristiques image.
    const LaneConfig m_config;                      ///< Configuration du pipeline.
    ImageSink& m_debug_sink;                         ///< Destination des traces.
    LaneMask m_mask;                                 ///< Étape masque binaire.
    PerspectiveView m_perspective;                   ///< Étape BEV.
    SlidingWindowSearch m_search;                    ///< Étape fenêtres glissantes.
    LaneOverlay m_overlay;                            ///< Étape rendu (déclarée après m_perspective).
  };
