#pragma once

#include "LaneModel.h"
#include "VideoCaracteristics.h"
#include "LaneConfig.h"

/*!
*  \brief Calcule le signal de pilotage a partir des deux polynomes de voie.
*
*  Reconstruit le cote manquant par decalage (si defaultLaneWidthPx > 0), puis
*  calcule offset lateral, offset normalise et rayon de courbure au bas de
*  l'image. Distingue aleas de la route (drapeaux) et invariants (SMART_ASSERT).
*/
class LaneGeometry {
    public:
        static LaneModel compute(LaneModel model,
                                 const VideoCaracteristics& video,
                                 const LaneConfig& config);
};
