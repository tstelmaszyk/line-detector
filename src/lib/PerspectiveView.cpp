/// @file
/// @brief Implémentation de PerspectiveView.

#include <opencv2/imgproc.hpp>

#include <cmath>
#include <vector>

#include "PerspectiveView.h"
#include "SmartAssert.h"

namespace
{

const float CENTER_DIVISOR = 2.0f;               ///< Diviseur pour le centre horizontal.
const double MIN_HOMOGRAPHY_DETERMINANT = 1e-6;  ///< Déterminant mini d'une homographie non dégénérée.

} // namespace

PerspectiveView::PerspectiveView( const VideoCaracteristics& p_video,
                                  const LaneConfig& p_config,
                                  ImageSink& p_debug_sink )
  : m_video_properties( p_video ),
    m_config( p_config ),
    m_debug_sink( p_debug_sink ),
    m_bev_size( p_video.image_size )
{
  const float width = static_cast< float >( m_video_properties.width_pixel );
  const float height = static_cast< float >( m_video_properties.height_pixel );

  const float top_y = m_config.src_top_y_ratio * height;
  const float top_half = m_config.src_top_width_ratio * width;
  const float bottom_y = m_config.src_bottom_y_ratio * height;
  const float bottom_half = m_config.src_bottom_width_ratio * width;
  const float center_x = width / CENTER_DIVISOR;

  // Quad source : trapèze, bords haut et bas tous deux réglables (caméras très
  // basses : les marquages sortent du cadre avant le bas de l'image, cf.
  // CLAUDE.md § Calibration BEV).
  m_src_quad = {
    ::cv::Point2f( center_x - top_half, top_y ),        // haut gauche
    ::cv::Point2f( center_x + top_half, top_y ),        // haut droit
    ::cv::Point2f( center_x + bottom_half, bottom_y ),  // bas droit
    ::cv::Point2f( center_x - bottom_half, bottom_y )   // bas gauche
  };

  // Rectangle BEV, avec marge latérale pour laisser respirer les virages.
  const float margin = m_config.bev_margin_ratio * width;
  const ::std::vector< ::cv::Point2f > dst_quad = {
    ::cv::Point2f( margin, 0.0f ),
    ::cv::Point2f( width - margin, 0.0f ),
    ::cv::Point2f( width - margin, height ),
    ::cv::Point2f( margin, height )
  };

  m_perspective_matrix = ::cv::getPerspectiveTransform( m_src_quad, dst_quad );
  m_perspective_matrix_inverse = ::cv::getPerspectiveTransform( dst_quad, m_src_quad );

  const double determinant = ::std::abs( ::cv::determinant( m_perspective_matrix ) );
  SMART_ASSERT( determinant > MIN_HOMOGRAPHY_DETERMINANT,
                "PerspectiveView: homographie degeneree (points colineaires ?)" );
}

void PerspectiveView::to_bev( const ::cv::Mat& p_src, ::cv::Mat& p_bev ) const
{
  const bool is_empty = p_src.empty();
  const bool size_matches = ( p_src.size() == m_video_properties.image_size );

  SMART_ASSERT( !is_empty, "to_bev: entree vide" );
  SMART_ASSERT( size_matches, "to_bev: taille != VideoCaracteristics" );

  ::cv::warpPerspective( p_src,
                         p_bev,
                         m_perspective_matrix,
                         m_bev_size,
                         ::cv::INTER_LINEAR,
                         ::cv::BORDER_CONSTANT,
                         ::cv::Scalar( 0 ) );
}

void PerspectiveView::warp_back( const ::cv::Mat& p_bev, ::cv::Mat& p_dst ) const
{
  const bool is_empty = p_bev.empty();

  SMART_ASSERT( !is_empty, "warp_back: entree vide" );

  ::cv::warpPerspective( p_bev,
                         p_dst,
                         m_perspective_matrix_inverse,
                         m_video_properties.image_size,
                         ::cv::INTER_LINEAR,
                         ::cv::BORDER_CONSTANT,
                         ::cv::Scalar( 0 ) );
}

::cv::Size PerspectiveView::bev_size() const
{
  return m_bev_size;
}

const ::std::vector< ::cv::Point2f >& PerspectiveView::source_quad() const
{
  return m_src_quad;
}
