/// @file
/// @brief Implémentation de LaneOverlay.

#include <opencv2/imgproc.hpp>

#include <cstdio>
#include <vector>

#include "LaneOverlay/LaneOverlay.h"
#include "SmartAssert/SmartAssert.h"

namespace
{

const int EXPECTED_CHANNELS = 3;                  ///< Nombre de canaux attendu (BGR).
const int HUD_BUFFER_SIZE = 128;                  ///< Taille du tampon texte du HUD.
const int HUD_TEXT_THICKNESS = 2;                 ///< Épaisseur du texte du HUD.
const double BASE_IMAGE_WEIGHT = 1.0;             ///< Poids de l'image de base dans la fusion.
const double LANE_OVERLAY_ALPHA = 0.3;            ///< Poids du calque de voie dans la fusion.
const double FUSION_GAMMA = 0.0;                  ///< Terme constant de la fusion.
const double HUD_FONT_SCALE = 1.0;                ///< Échelle de la police du HUD.
const ::cv::Point HUD_TEXT_ORIGIN( 20, 40 );      ///< Origine du texte du HUD (pixels).
const ::cv::Scalar COLOR_LANE_FILL( 0, 255, 0 );  ///< Vert : remplissage de la voie.
const ::cv::Scalar COLOR_HUD_TEXT( 0, 0, 255 );   ///< Rouge : texte du HUD.

} // namespace

LaneOverlay::LaneOverlay( const PerspectiveView& p_perspective, ImageSink& p_debug_sink )
  : m_perspective( p_perspective ),
    m_debug_sink( p_debug_sink )
{
}

void LaneOverlay::render( const ::cv::Mat& p_original_bgr,
                          const LaneModel& p_model,
                          ::cv::Mat& p_output ) const
{
  const bool is_empty = p_original_bgr.empty();
  const int channel_count = p_original_bgr.channels();

  SMART_ASSERT( !is_empty, "render: image d'origine vide" );
  SMART_ASSERT( EXPECTED_CHANNELS == channel_count, "render: attend une image BGR" );

  p_output = p_original_bgr.clone();

  if ( !p_model.lane_detected )
    {
    m_debug_sink.save( "debug_05_overlay.jpg", p_output );
    return;
    }

  const ::cv::Size bev = m_perspective.bev_size();
  ::cv::Mat lane_bev( bev, CV_8UC3, ::cv::Scalar( 0, 0, 0 ) );

  // Polygone entre les deux polynômes, échantillonné le long de y.
  ::std::vector< ::cv::Point > lane_polygon;

  for ( int y = 0; y < bev.height; ++y )
    {
    const int x_left = ::cvRound( p_model.left.eval_at( y ) );
    lane_polygon.emplace_back( x_left, y );
    }

  for ( int y = bev.height - 1; y >= 0; --y )
    {
    const int x_right = ::cvRound( p_model.right.eval_at( y ) );
    lane_polygon.emplace_back( x_right, y );
    }

  const ::std::vector< ::std::vector< ::cv::Point > > polygons = { lane_polygon };
  ::cv::fillPoly( lane_bev, polygons, COLOR_LANE_FILL );

  // Retour en perspective image et fusion.
  ::cv::Mat lane_image;
  m_perspective.warp_back( lane_bev, lane_image );

  // src1 == dst est intentionnel et supporté par addWeighted (aliasing OK).
  ::cv::addWeighted( p_output,
                     BASE_IMAGE_WEIGHT,
                     lane_image,
                     LANE_OVERLAY_ALPHA,
                     FUSION_GAMMA,
                     p_output );

  // HUD : offset normalisé et rayon de courbure.
  char hud[HUD_BUFFER_SIZE];
  ::std::snprintf( hud,
                   sizeof( hud ),
                   "offset=%.2f  R=%.0fpx",
                   p_model.normalized_offset,
                   p_model.curvature_radius_px );

  ::cv::putText( p_output,
                 hud,
                 HUD_TEXT_ORIGIN,
                 ::cv::FONT_HERSHEY_SIMPLEX,
                 HUD_FONT_SCALE,
                 COLOR_HUD_TEXT,
                 HUD_TEXT_THICKNESS );

  m_debug_sink.save( "debug_05_overlay.jpg", p_output );
}
