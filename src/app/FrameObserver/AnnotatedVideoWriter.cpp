/// @file
/// @brief Implémentation de AnnotatedVideoWriter.

#include <string>
#include <utility>

#include "FrameObserver/AnnotatedVideoWriter.h"
#include "LaneModel/LaneModel.h"

namespace
{

const char FOURCC_FIRST = 'M';   ///< Codec MJPG : 1er caractère.
const char FOURCC_SECOND = 'J';  ///< Codec MJPG : 2e caractère.
const char FOURCC_THIRD = 'P';   ///< Codec MJPG : 3e caractère.
const char FOURCC_FOURTH = 'G';  ///< Codec MJPG : 4e caractère.

} // namespace

AnnotatedVideoWriter::AnnotatedVideoWriter( ::std::string p_output_path, double p_frames_per_second )
  : m_output_path( ::std::move( p_output_path ) ),
    m_frames_per_second( p_frames_per_second ),
    m_video_writer(),
    m_has_failed( false )
{
}

AnnotatedVideoWriter::~AnnotatedVideoWriter()
{
  m_video_writer.release();
}

void AnnotatedVideoWriter::on_frame( int p_frame_index,
                                     const LaneModel& p_model,
                                     const ::cv::Mat& p_annotated_frame,
                                     double p_compute_ms,
                                     double p_render_ms )
{
  (void) p_frame_index;
  (void) p_model;
  (void) p_compute_ms;
  (void) p_render_ms;

  // Un echec est definitif : on n'ecrit plus rien et on ne retente pas l'ouverture.
  if ( true == m_has_failed )
    {
    return;
    }

  const bool frame_is_empty = p_annotated_frame.empty();

  if ( frame_is_empty )
    {
    m_has_failed = true;
    return;
    }

  // Ouverture paresseuse : la taille vient de la premiere frame reelle.
  const bool already_opened = m_video_writer.isOpened();

  if ( !already_opened )
    {
    const int fourcc = ::cv::VideoWriter::fourcc( FOURCC_FIRST, FOURCC_SECOND, FOURCC_THIRD, FOURCC_FOURTH );
    const ::cv::Size frame_size = p_annotated_frame.size();
    m_video_writer.open( m_output_path, fourcc, m_frames_per_second, frame_size );

    const bool opened_now = m_video_writer.isOpened();

    if ( !opened_now )
      {
      m_has_failed = true;
      return;
      }
    }

  m_video_writer.write( p_annotated_frame );
}

bool AnnotatedVideoWriter::has_failed() const
{
  return m_has_failed;
}
