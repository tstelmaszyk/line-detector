#pragma once

/// @file
/// @brief Caractéristiques géométriques de l'image de référence.

#include <opencv2/core.hpp>

#include "projectTypes.h"

/// @brief Dimensions de l'image, source unique de géométrie du pipeline.
struct VideoCaracteristics
  {
  ::cv::Size image_size;        ///< Taille de l'image (pixels).
  DimensionImage width_pixel;   ///< Largeur de l'image en pixels.
  DimensionImage height_pixel;  ///< Hauteur de l'image en pixels.

  /// @brief Construit les caractéristiques depuis une image de référence.
  /// @param p_reference_frame Image de référence (non vide).
  explicit VideoCaracteristics( const ::cv::Mat& p_reference_frame )
    : image_size( p_reference_frame.size() ),
      width_pixel( p_reference_frame.size().width ),
      height_pixel( p_reference_frame.size().height )
    {
    }
  };
