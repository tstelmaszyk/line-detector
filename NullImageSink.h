#pragma once

/// @file
/// @brief Sink no-op : n'écrit rien (défaut quand le debug est désactivé).

#include <opencv2/core.hpp>

#include <string>

#include "ImageSink.h"

/// @brief Sink no-op : n'écrit rien. Renvoie true (rien à écrire n'est pas un échec).
class NullImageSink : public ImageSink
  {
  public:
    /// @brief Ne fait rien.
    /// @param p_name  Ignoré.
    /// @param p_frame Ignoré.
    /// @return Toujours true.
    bool save( const ::std::string& p_name, const ::cv::Mat& p_frame ) override
      {
      (void) p_name;
      (void) p_frame;
      return true;
      }
  };
