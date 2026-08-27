/// @file
/// @brief Implémentation de CaptureFrameSource.

#include <memory>
#include <string>

#include "FrameSource/CaptureFrameSource.h"

CaptureFrameSource::CaptureFrameSource()
  : m_capture()
{
}

::std::unique_ptr< CaptureFrameSource > CaptureFrameSource::from_file( const ::std::string& p_video_path )
{
  ::std::unique_ptr< CaptureFrameSource > source( new CaptureFrameSource() );
  source->m_capture.open( p_video_path );
  return source;
}

::std::unique_ptr< CaptureFrameSource > CaptureFrameSource::from_camera( CameraIndex p_camera_index )
{
  ::std::unique_ptr< CaptureFrameSource > source( new CaptureFrameSource() );
  source->m_capture.open( p_camera_index );
  return source;
}

bool CaptureFrameSource::is_opened() const
{
  const bool opened = m_capture.isOpened();
  return opened;
}

bool CaptureFrameSource::read( ::cv::Mat& p_frame )
{
  const bool opened = m_capture.isOpened();

  if ( !opened )
    {
    return false;
    }

  const bool read_ok = m_capture.read( p_frame );
  return read_ok;
}
