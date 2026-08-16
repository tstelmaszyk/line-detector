/// @file
/// @brief Implémentation de StillImageFrameSource.

#include <opencv2/imgcodecs.hpp>

#include <string>

#include "StillImageFrameSource.h"

StillImageFrameSource::StillImageFrameSource( const ::std::string& p_image_path )
  : m_image(),
    m_already_delivered( false )
{
  m_image = ::cv::imread( p_image_path, ::cv::IMREAD_COLOR );
}

bool StillImageFrameSource::is_opened() const
{
  const bool is_empty = m_image.empty();
  return !is_empty;
}

bool StillImageFrameSource::read( ::cv::Mat& p_frame )
{
  const bool is_empty = m_image.empty();

  if ( is_empty || m_already_delivered )
    {
    return false;
    }

  p_frame = m_image.clone();
  m_already_delivered = true;
  return true;
}
