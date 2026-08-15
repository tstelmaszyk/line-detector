/// @file
/// @brief Implémentation de ResultImageWriter.

#include <string>
#include <utility>

#include "LaneModel.h"
#include "ResultImageWriter.h"

ResultImageWriter::ResultImageWriter( ImageSink& p_image_sink, ::std::string p_file_name )
  : m_image_sink( p_image_sink ),
    m_file_name( ::std::move( p_file_name ) ),
    m_has_failed( false )
{
}

void ResultImageWriter::on_frame( int p_frame_index,
                                  const LaneModel& p_model,
                                  const ::cv::Mat& p_annotated_frame,
                                  double p_elapsed_ms )
{
  (void) p_frame_index;
  (void) p_model;
  (void) p_elapsed_ms;

  const bool saved = m_image_sink.save( m_file_name, p_annotated_frame );

  if ( !saved )
    {
    m_has_failed = true;
    }
}

bool ResultImageWriter::has_failed() const
{
  return m_has_failed;
}
