#pragma once

/// @file
/// @brief Masque binaire des marquages (blanc + jaune + bords Sobel).

#include <opencv2/core.hpp>

#include "ImageSink.h"
#include "LaneConfig.h"
#include "VideoCaracteristics.h"

/// @brief Produit un binaire des marquages : blanc (luminance) + jaune (HSV) +
/// bords (Sobel x), combinés par OU. Entrée BGR (cv::imread), sortie CV_8UC1 {0,255}.
class LaneMask
  {
  public:
    /// @brief Construit le masqueur.
    /// @param p_video      Caractéristiques image.
    /// @param p_config     Configuration du pipeline.
    /// @param p_debug_sink Destination des traces de debug.
    LaneMask( const VideoCaracteristics& p_video,
              const LaneConfig& p_config,
              ImageSink& p_debug_sink );

    /// @brief Calcule le masque binaire des marquages.
    /// @param p_bgr    Image d'entrée BGR (3 canaux).
    /// @param p_binary Masque de sortie (CV_8UC1, {0,255}).
    void compute( const ::cv::Mat& p_bgr, ::cv::Mat& p_binary ) const;

  private:
    const VideoCaracteristics m_video_properties;  ///< Caractéristiques image.
    const LaneConfig m_config;                      ///< Configuration du pipeline.
    ImageSink& m_debug_sink;                         ///< Destination des traces.
  };
