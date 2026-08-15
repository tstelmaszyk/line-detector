/// @file
/// @brief Implémentation de PipelineRunner.

#include <chrono>
#include <cstdlib>
#include <vector>

#include "LaneModel.h"
#include "PipelineRunner.h"

namespace
{

const double MICROSECONDS_PER_MILLISECOND = 1000.0;  ///< Conversion µs -> ms.

} // namespace

PipelineRunner::PipelineRunner( FrameSource& p_frame_source,
                                const DetectLines& p_detector,
                                const ::std::vector< FrameObserver* >& p_observers,
                                const ::std::atomic< bool >& p_stop_requested )
  : m_frame_source( p_frame_source ),
    m_detector( p_detector ),
    m_observers( p_observers ),
    m_stop_requested( p_stop_requested )
{
}

void PipelineRunner::process_frame( const ::cv::Mat& p_frame, int p_frame_index, RunStats& p_stats )
{
  const ::std::chrono::steady_clock::time_point start_time = ::std::chrono::steady_clock::now();

  ::cv::Mat annotated_frame;
  const LaneModel model = m_detector.draw_lines( p_frame, annotated_frame );

  const ::std::chrono::steady_clock::time_point end_time = ::std::chrono::steady_clock::now();
  const ::std::chrono::microseconds elapsed_us =
    ::std::chrono::duration_cast< ::std::chrono::microseconds >( end_time - start_time );
  const double elapsed_ms = static_cast< double >( elapsed_us.count() ) / MICROSECONDS_PER_MILLISECOND;

  for ( FrameObserver* observer : m_observers )
    {
    if ( nullptr != observer )
      {
      observer->on_frame( p_frame_index, model, annotated_frame, elapsed_ms );
      }
    }

  ++p_stats.frame_count;
  p_stats.total_ms = p_stats.total_ms + elapsed_ms;

  if ( model.lane_detected )
    {
    ++p_stats.detected_count;
    }

  if ( model.reconstructed )
    {
    ++p_stats.reconstructed_count;
    }
}

int PipelineRunner::run( const ::cv::Mat& p_first_frame, RunStats& p_stats )
{
  ::cv::Mat current_frame = p_first_frame;
  int frame_index = 0;

  while ( true )
    {
    const bool frame_is_empty = current_frame.empty();

    if ( frame_is_empty )
      {
      break;
      }

    process_frame( current_frame, frame_index, p_stats );
    ++frame_index;

    // Un arret demande est constate apres la frame en cours, pour ne pas la perdre.
    const bool stop_after_frame = m_stop_requested.load();

    if ( stop_after_frame )
      {
      break;
      }

    ::cv::Mat next_frame;
    const bool read_ok = m_frame_source.read( next_frame );

    if ( !read_ok )
      {
      break;
      }

    current_frame = next_frame;
    }

  const bool any_frame_processed = ( 0 < p_stats.frame_count );

  if ( !any_frame_processed )
    {
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
