#pragma once

/// @file
/// @brief Vue de dessus (bird's eye view) et transformations perspective.

#include <opencv2/core.hpp>

#include <vector>

#include "ImageSink/ImageSink.h"
#include "LaneConfig/LaneConfig.h"
#include "VideoCaracteristics/VideoCaracteristics.h"

/// @brief Construit les homographies directe et inverse d'un quadrilatère source
/// (trapèze) vers un rectangle BEV plein cadre.
class PerspectiveView
  {
  public:
    /// @brief Construit les transformations perspective.
    /// @param p_video      Caractéristiques image.
    /// @param p_config     Configuration (ratios de calibration BEV).
    /// @param p_debug_sink Destination des traces de debug.
    PerspectiveView( const VideoCaracteristics& p_video,
                     const LaneConfig& p_config,
                     ImageSink& p_debug_sink );

    /// @brief Transforme l'image vers la vue de dessus.
    /// @param p_src Image source.
    /// @param p_bev Image BEV de sortie.
    void to_bev( const ::cv::Mat& p_src, ::cv::Mat& p_bev ) const;

    /// @brief Transforme une image BEV vers la perspective d'origine.
    /// @param p_bev Image BEV.
    /// @param p_dst Image en perspective de sortie.
    void warp_back( const ::cv::Mat& p_bev, ::cv::Mat& p_dst ) const;

    /// @brief Taille de la vue BEV.
    /// @return Taille BEV (pixels).
    ::cv::Size bev_size() const;

    /// @brief Quadrilatère source (trapèze) de la transformation.
    /// @return Les 4 sommets du quad source.
    const ::std::vector< ::cv::Point2f >& source_quad() const;

  private:
    const VideoCaracteristics m_video_properties;  ///< Caractéristiques image.
    const LaneConfig m_config;                      ///< Configuration du pipeline.
    ImageSink& m_debug_sink;                         ///< Destination des traces.
    ::std::vector< ::cv::Point2f > m_src_quad;      ///< Quad source (trapèze).
    ::cv::Size m_bev_size;                           ///< Taille de la vue BEV.
    ::cv::Mat m_perspective_matrix;                  ///< Homographie src -> BEV.
    ::cv::Mat m_perspective_matrix_inverse;          ///< Homographie BEV -> src.
  };
