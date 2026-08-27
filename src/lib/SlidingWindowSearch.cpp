/// @file
/// @brief Implémentation de SlidingWindowSearch.

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <vector>

#include "SlidingWindowSearch.h"
#include "SmartAssert.h"

namespace
{

const int HALF_DIVISOR = 2;                         ///< Diviseur pour la moitié de largeur.
const ::cv::Vec3b COLOR_LEFT_PIXELS( 0, 0, 255 );   ///< Rouge : pixels gauche (debug).
const ::cv::Vec3b COLOR_RIGHT_PIXELS( 255, 0, 0 );  ///< Bleu : pixels droite (debug).

} // namespace

SlidingWindowSearch::SlidingWindowSearch( const VideoCaracteristics& p_video,
                                          const LaneConfig& p_config,
                                          ImageSink& p_debug_sink )
  : m_video_properties( p_video ),
    m_config( p_config ),
    m_debug_sink( p_debug_sink )
{
}

LanePixels SlidingWindowSearch::search( const ::cv::Mat& p_bev ) const
{
  const bool is_empty = p_bev.empty();
  const bool is_single_channel = ( p_bev.type() == CV_8UC1 );

  SMART_ASSERT( !is_empty, "search: BEV vide" );
  SMART_ASSERT( is_single_channel, "search: attend un binaire mono-canal" );

  const int height_px = p_bev.rows;
  const int width_px = p_bev.cols;
  const int half_width = width_px / HALF_DIVISOR;

  // Histogramme des colonnes sur une bande basse étroite : la base reflète la
  // position des lignes au tout-bas (où commence la 1re fenêtre).
  const int band_height = static_cast< int >( m_config.histogram_band_ratio * height_px );
  const int hist_top = ::std::max( 0, height_px - band_height );

  ::std::vector< int > column_histogram( width_px, 0 );

  for ( int row = hist_top; row < height_px; ++row )
    {
    const uchar* pixel_row = p_bev.ptr< uchar >( row );

    for ( int col = 0; col < width_px; ++col )
      {
      if ( pixel_row[col] > 0 )
        {
        column_histogram[col]++;
        }
      }
    }

  // Deux pics : gauche dans [0, W/2), droite dans [W/2, W).
  int left_base = 0;
  int right_base = half_width;
  int left_max = -1;
  int right_max = -1;

  for ( int col = 0; col < half_width; ++col )
    {
    if ( column_histogram[col] > left_max )
      {
      left_max = column_histogram[col];
      left_base = col;
      }
    }

  for ( int col = half_width; col < width_px; ++col )
    {
    if ( column_histogram[col] > right_max )
      {
      right_max = column_histogram[col];
      right_base = col;
      }
    }

  LanePixels pixels;

  const int window_count = m_config.window_count;
  const int standard_margin = m_config.window_margin;
  const int min_pixels = m_config.window_min_pix;
  const int window_height = ::std::max( 1, height_px / window_count );

  int left_center = left_base;
  int right_center = right_base;

  for ( int window_index = 0; window_index < window_count; ++window_index )
    {
    const int y_low = ::std::max( 0, height_px - ( ( window_index + 1 ) * window_height ) );
    const int y_high = height_px - ( window_index * window_height );
    const int current_margin = ( 0 == window_index ) ? m_config.first_window_margin : standard_margin;

    // Fenêtre gauche.
    int left_sum_x = 0;
    int left_pixel_count = 0;

    for ( int row = y_low; row < y_high; ++row )
      {
      const uchar* pixel_row = p_bev.ptr< uchar >( row );
      const int col_begin = ::std::max( 0, left_center - current_margin );
      const int col_end = ::std::min( width_px, left_center + current_margin );

      for ( int col = col_begin; col < col_end; ++col )
        {
        if ( pixel_row[col] > 0 )
          {
          pixels.left.emplace_back( col, row );
          left_sum_x += col;
          left_pixel_count++;
          }
        }
      }

    if ( left_pixel_count > min_pixels )
      {
      left_center = left_sum_x / left_pixel_count;
      }

    // Fenêtre droite.
    int right_sum_x = 0;
    int right_pixel_count = 0;

    for ( int row = y_low; row < y_high; ++row )
      {
      const uchar* pixel_row = p_bev.ptr< uchar >( row );
      const int col_begin = ::std::max( 0, right_center - current_margin );
      const int col_end = ::std::min( width_px, right_center + current_margin );

      for ( int col = col_begin; col < col_end; ++col )
        {
        if ( pixel_row[col] > 0 )
          {
          pixels.right.emplace_back( col, row );
          right_sum_x += col;
          right_pixel_count++;
          }
        }
      }

    if ( right_pixel_count > min_pixels )
      {
      right_center = right_sum_x / right_pixel_count;
      }
    }

  // Trace debug : pixels gauche en rouge, droite en bleu. Construction sautee
  // si aucun sink ne consomme le resultat (cout cvtColor + ecriture pixel par
  // pixel non negligeable, payee sinon a chaque frame meme sans debug).
  if ( m_debug_sink.is_enabled() )
    {
    ::cv::Mat debug_image;
    ::cv::cvtColor( p_bev, debug_image, ::cv::COLOR_GRAY2BGR );

    for ( const ::cv::Point& point : pixels.left )
      {
      debug_image.at< ::cv::Vec3b >( point ) = COLOR_LEFT_PIXELS;
      }

    for ( const ::cv::Point& point : pixels.right )
      {
      debug_image.at< ::cv::Vec3b >( point ) = COLOR_RIGHT_PIXELS;
      }

    m_debug_sink.save( "debug_03_windows.jpg", debug_image );
    }

  return pixels;
}
