#pragma once

/// @file
/// @brief Calcul du signal de pilotage à partir des deux polynômes de voie.

#include "LaneConfig.h"
#include "LaneModel.h"
#include "VideoCaracteristics.h"

/// @brief Calcule le signal de pilotage (offset, courbure) à partir du modèle de
/// voie. Reconstruit le côté manquant par décalage si default_lane_width_px > 0.
/// Distingue aléas de la route (drapeaux) et invariants (SMART_ASSERT).
class LaneGeometry
  {
  public:
    /// @brief Calcule offset et courbure et complète le LaneModel.
    /// @param p_model  Modèle de voie (polynômes ajustés).
    /// @param p_video  Caractéristiques image.
    /// @param p_config Configuration du pipeline.
    /// @return Modèle complété (signal de pilotage).
    static LaneModel compute( LaneModel p_model,
                              const VideoCaracteristics& p_video,
                              const LaneConfig& p_config );
  };
