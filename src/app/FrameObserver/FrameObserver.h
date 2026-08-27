#pragma once

/// @file
/// @brief Interface de destination d'un résultat de détection, frame par frame.

#include <opencv2/core.hpp>

#include "projectTypes.h"

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
    /// @param p_annotated_frame Image annotée ; **vide** si aucun observateur
    ///                          n'a déclaré en avoir besoin.
    /// @param p_compute_ms      Durée du calcul de la frame (millisecondes).
    /// @param p_render_ms       Durée du rendu de la frame (0 si non rendu).
    virtual void on_frame( FrameIndex p_frame_index,
                           const LaneModel& p_model,
                           const ::cv::Mat& p_annotated_frame,
                           double p_compute_ms,
                           double p_render_ms ) = 0;

    /// @brief Indique si l'observateur est dans un état d'échec définitif.
    /// @return false par défaut ; les observateurs qui écrivent en sortie le redéfinissent.
    virtual bool has_fatal_error() const { return false; }

    /// @brief Indique si cet observateur exploite l'image annotée.
    ///
    /// Un observateur qui rend false recevra un ::cv::Mat vide et ne doit pas
    /// le lire. Le défaut est true : se tromper dans ce sens coûte du temps
    /// CPU, se tromper dans l'autre coûte un Mat vide déréférencé.
    /// @return true par défaut.
    virtual bool needs_annotated_frame() const { return true; }
  };
