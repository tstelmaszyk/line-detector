/// @file
/// @brief Implémentation de LaneGeometry.

#include <cmath>

#include "LaneGeometry.h"
#include "SmartAssert.h"

namespace
{

const double CENTER_DIVISOR = 2.0;              ///< Diviseur pour un centre / une moitié.
const double MIN_LANE_WIDTH_PX = 1.0;           ///< Largeur de voie mini plausible (pixels).
const double SECOND_DERIVATIVE_FACTOR = 2.0;    ///< f''(y) = 2a pour x = a*y^2 + b*y + c.
const double MIN_CURVATURE_DENOMINATOR = 1e-9;  ///< En dessous : voie considérée droite.
const double STRAIGHT_LANE_RADIUS_PX = 1e12;    ///< Rayon d'une voie quasi droite.
const double CURVATURE_EXPONENT = 1.5;          ///< Exposant de la formule de courbure.

} // namespace

LaneModel LaneGeometry::compute( LaneModel p_model,
                                 const VideoCaracteristics& p_video,
                                 const LaneConfig& p_config )
{
  const double y_max = static_cast< double >( p_video.height_pixel ) - 1.0;
  const double image_center_x = static_cast< double >( p_video.width_pixel ) / CENTER_DIVISOR;

  // Reconstruction du côté manquant par décalage d'une largeur de voie.
  const bool can_reconstruct = ( p_config.default_lane_width_px > 0.0 );

  if ( p_model.left.valid && !p_model.right.valid && can_reconstruct )
    {
    p_model.right = p_model.left;
    p_model.right.constant_coefficient += p_config.default_lane_width_px;
    p_model.reconstructed = true;
    }
  else if ( p_model.right.valid && !p_model.left.valid && can_reconstruct )
    {
    p_model.left = p_model.right;
    p_model.left.constant_coefficient -= p_config.default_lane_width_px;
    p_model.reconstructed = true;
    }

  if ( !p_model.left.valid || !p_model.right.valid )
    {
    p_model.lane_detected = false;
    p_model.lateral_offset_px = 0.0;
    p_model.normalized_offset = 0.0;
    p_model.curvature_radius_px = 0.0;
    return p_model;
    }

  const double x_left = p_model.left.eval_at( y_max );
  const double x_right = p_model.right.eval_at( y_max );
  const double lane_width = x_right - x_left;

  // Sanité : largeur de voie positive plausible (aléa -> drapeau, pas d'assert).
  if ( lane_width <= MIN_LANE_WIDTH_PX )
    {
    p_model.lane_detected = false;
    p_model.lateral_offset_px = 0.0;
    p_model.normalized_offset = 0.0;
    p_model.curvature_radius_px = 0.0;
    return p_model;
    }

  const double lane_center = ( x_left + x_right ) / CENTER_DIVISOR;
  const double half_width = lane_width / CENTER_DIVISOR;

  p_model.lateral_offset_px = image_center_x - lane_center;
  p_model.normalized_offset = p_model.lateral_offset_px / half_width;

  // Rayon de courbure du côté gauche (les deux sont ~parallèles en BEV).
  const double quadratic = p_model.left.quadratic_coefficient;
  const double linear = p_model.left.linear_coefficient;
  const double denominator = ::std::abs( SECOND_DERIVATIVE_FACTOR * quadratic );

  if ( denominator < MIN_CURVATURE_DENOMINATOR )
    {
    p_model.curvature_radius_px = STRAIGHT_LANE_RADIUS_PX;
    }
  else
    {
    const double slope = ( SECOND_DERIVATIVE_FACTOR * quadratic * y_max ) + linear;
    p_model.curvature_radius_px =
      ::std::pow( 1.0 + ( slope * slope ), CURVATURE_EXPONENT ) / denominator;
    }

  const bool offset_is_finite = ::std::isfinite( p_model.normalized_offset );
  SMART_ASSERT( offset_is_finite, "normalized_offset non fini" );

  p_model.lane_detected = true;
  return p_model;
}
