/// @file
/// @brief Implémentation de LaneMask.

#include <opencv2/imgproc.hpp>

#include "LaneMask/LaneMask.h"
#include "SmartAssert/SmartAssert.h"

namespace
{

const int MASK_ON_VALUE = 255;      ///< Valeur "marquage présent" du masque binaire.
const int HSV_SATURATION_MIN = 80;  ///< Saturation mini du jaune (HSV).
const int HSV_VALUE_MIN = 80;       ///< Valeur mini du jaune (HSV).
const int HSV_CHANNEL_MAX = 255;    ///< Valeur maxi d'un canal HSV.
const int EXPECTED_CHANNELS = 3;    ///< Nombre de canaux attendu (BGR).

} // namespace

LaneMask::LaneMask( const VideoCaracteristics& p_video,
                    const LaneConfig& p_config,
                    ImageSink& p_debug_sink )
  : m_video_properties( p_video ),
    m_config( p_config ),
    m_debug_sink( p_debug_sink )
{
}

void LaneMask::compute( const ::cv::Mat& p_bgr, ::cv::Mat& p_binary ) const
{
  const bool is_empty = p_bgr.empty();
  const int channel_count = p_bgr.channels();
  const bool size_matches = ( p_bgr.size() == m_video_properties.image_size );

  SMART_ASSERT( !is_empty, "LaneMask: image d'entree vide" );
  SMART_ASSERT( EXPECTED_CHANNELS == channel_count, "LaneMask: attend une image BGR 3 canaux" );
  SMART_ASSERT( size_matches, "LaneMask: taille != VideoCaracteristics" );

  // 1. Marquage blanc : forte luminance. BGR (imread charge en BGR), PAS RGB.
  ::cv::Mat gray;
  ::cv::cvtColor( p_bgr, gray, ::cv::COLOR_BGR2GRAY );

  ::cv::Mat white;
  ::cv::threshold( gray, white, m_config.white_threshold, MASK_ON_VALUE, ::cv::THRESH_BINARY );

  // 2. Marquage jaune : plage de teinte en HSV.
  ::cv::Mat hsv;
  ::cv::cvtColor( p_bgr, hsv, ::cv::COLOR_BGR2HSV );

  ::cv::Mat yellow;
  const ::cv::Scalar yellow_lower( m_config.yellow_hue[0], HSV_SATURATION_MIN, HSV_VALUE_MIN );
  const ::cv::Scalar yellow_upper( m_config.yellow_hue[1], HSV_CHANNEL_MAX, HSV_CHANNEL_MAX );
  ::cv::inRange( hsv, yellow_lower, yellow_upper, yellow );

  // 3. Gradient horizontal (Sobel x), avec flou préalable contre la texture asphalte.
  ::cv::Mat gray_blur;
  const ::cv::Size blur_size( m_config.blur_kernel, m_config.blur_kernel );
  ::cv::GaussianBlur( gray, gray_blur, blur_size, 0 );

  ::cv::Mat sobel_x;
  ::cv::Sobel( gray_blur, sobel_x, CV_16S, 1, 0, m_config.sobel_kernel );

  ::cv::Mat sobel_abs;
  ::cv::convertScaleAbs( sobel_x, sobel_abs );

  ::cv::Mat sobel_bin;
  const ::cv::Scalar sobel_lower( m_config.sobel_thresh_low );
  const ::cv::Scalar sobel_upper( m_config.sobel_thresh_high );
  ::cv::inRange( sobel_abs, sobel_lower, sobel_upper, sobel_bin );

  // Le grain de l'asphalte survit au flou dans le champ proche (bas) : on ne
  // garde le Sobel que dans le champ lointain (haut).
  const int sobel_cutoff = static_cast< int >( m_config.sobel_near_cutoff_ratio * sobel_bin.rows );

  if ( ( sobel_cutoff >= 0 ) && ( sobel_cutoff < sobel_bin.rows ) )
    {
    sobel_bin.rowRange( sobel_cutoff, sobel_bin.rows ).setTo( 0 );
    }

  // 4. Combinaison OU des trois masques, puis ouverture morpho.
  p_binary = white | yellow | sobel_bin;

  const ::cv::Size morph_size( m_config.morph_kernel, m_config.morph_kernel );
  const ::cv::Mat kernel = ::cv::getStructuringElement( ::cv::MORPH_ELLIPSE, morph_size );
  ::cv::morphologyEx( p_binary, p_binary, ::cv::MORPH_OPEN, kernel );

  const bool is_single_channel = ( p_binary.type() == CV_8UC1 );
  SMART_ASSERT( is_single_channel, "LaneMask: sortie non binaire mono-canal" );

  m_debug_sink.save( "debug_01_mask.jpg", p_binary );
}
