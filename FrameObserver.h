#pragma once

/// @file
/// @brief Interface de destination d'un résultat de détection, frame par frame.

#include <opencv2/core.hpp>

struct LaneModel;

/// @brief Reçoit le résultat de détection d'une frame.
///
/// Sorties « dev/test » du harnais (log, vidéo annotée, image résultat). La
/// bibliothèque de détection ne connaît pas ce type.
class FrameObserver
  {
  public:
    /// @brief Destructeur virtuel par défaut.
    virtual ~FrameObserver() = default;

    /// @brief Notifie le résultat d'une frame.
    /// @param p_frame_index     Index de la frame (0 pour la première).
    /// @param p_model           Modèle de voie calculé.
    /// @param p_annotated_frame Image annotée produite par le pipeline.
    /// @param p_elapsed_ms      Durée de traitement de la frame (millisecondes).
    virtual void on_frame( int p_frame_index,
                           const LaneModel& p_model,
                           const ::cv::Mat& p_annotated_frame,
                           double p_elapsed_ms ) = 0;
  };
