#pragma once

/// @file
/// @brief Recherche des pixels de voie par histogramme + fenêtres glissantes.

#include <opencv2/core.hpp>

#include <vector>

#include "ImageSink/ImageSink.h"
#include "LaneConfig/LaneConfig.h"
#include "VideoCaracteristics/VideoCaracteristics.h"

/// @brief Pixels de voie séparés gauche/droite (coordonnées en pixels BEV).
struct LanePixels
  {
  ::std::vector< ::cv::Point > left;   ///< Pixels du côté gauche.
  ::std::vector< ::cv::Point > right;  ///< Pixels du côté droit.
  };

/// @brief Histogramme (bande basse) pour trouver les deux bases, puis fenêtres
/// glissantes de bas en haut pour collecter les pixels de chaque côté. Une
/// fenêtre sans assez de pixels NE bouge PAS.
class SlidingWindowSearch
  {
  public:
    /// @brief Construit le chercheur.
    /// @param p_video      Caractéristiques image.
    /// @param p_config     Configuration (fenêtres glissantes).
    /// @param p_debug_sink Destination des traces de debug.
    SlidingWindowSearch( const VideoCaracteristics& p_video,
                         const LaneConfig& p_config,
                         ImageSink& p_debug_sink );

    /// @brief Recherche les pixels de voie gauche/droite.
    /// @param p_bev Image BEV binaire (CV_8UC1).
    /// @return Pixels séparés gauche/droite.
    LanePixels search( const ::cv::Mat& p_bev ) const;

  private:
    const VideoCaracteristics m_video_properties;  ///< Caractéristiques image.
    const LaneConfig m_config;                      ///< Configuration du pipeline.
    ImageSink& m_debug_sink;                         ///< Destination des traces.
  };
