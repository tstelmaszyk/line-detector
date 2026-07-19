/// @file
/// @brief Implémentation de DiskImageSink.

#include <opencv2/imgcodecs.hpp>

#include <string>
#include <utility>

#include "DiskImageSink.h"

namespace
{

const ::std::string PATH_SEPARATOR = "/";  ///< Séparateur de chemin de sortie.

} // namespace

DiskImageSink::DiskImageSink( ::std::string p_output_dir )
  : m_output_dir( ::std::move( p_output_dir ) )
{
}

bool DiskImageSink::save( const ::std::string& p_name, const ::cv::Mat& p_frame )
{
  const ::std::string full_path = m_output_dir + PATH_SEPARATOR + p_name;

  return ::cv::imwrite( full_path, p_frame );
}
