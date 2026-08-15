#pragma once

/// @file
/// @brief Observateur écrivant une ligne exploitable en CSV par frame.

#include <opencv2/core.hpp>

#include <ostream>

#include "FrameObserver.h"

struct LaneModel;

/// @brief Écrit le signal de pilotage, une ligne par frame, sur un flux texte.
class LaneModelLogger : public FrameObserver
  {
  public:
    /// @brief Construit le logger.
    /// @param p_output_stream Flux de sortie (std::cout, ou un flux de test).
    explicit LaneModelLogger( ::std::ostream& p_output_stream );

    /// @brief Écrit l'en-tête au premier appel, puis une ligne par frame.
    /// @param p_frame_index     Index de la frame.
    /// @param p_model           Modèle de voie calculé.
    /// @param p_annotated_frame Image annotée (inutilisée ici).
    /// @param p_elapsed_ms      Durée de traitement de la frame (millisecondes).
    void on_frame( int p_frame_index,
                   const LaneModel& p_model,
                   const ::cv::Mat& p_annotated_frame,
                   double p_elapsed_ms ) override;

  private:
    ::std::ostream& m_output_stream;  ///< Flux de sortie.
    bool m_header_written;            ///< true une fois l'en-tête écrit.
  };
