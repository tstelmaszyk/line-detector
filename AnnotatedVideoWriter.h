#pragma once

/// @file
/// @brief Observateur écrivant la séquence annotée dans un fichier vidéo.

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <string>

#include "FrameObserver.h"

struct LaneModel;

/// @brief Écrit les frames annotées dans un fichier vidéo.
///
/// Le writer est ouvert **paresseusement** à la première frame : la taille des
/// images n'est connue qu'à ce moment (elle vient de la frame réelle, pas des
/// métadonnées de la source).
class AnnotatedVideoWriter : public FrameObserver
  {
  public:
    /// @brief Construit l'observateur sans ouvrir le fichier.
    /// @param p_output_path        Chemin du fichier vidéo à écrire.
    /// @param p_frames_per_second  Cadence déclarée dans le fichier.
    AnnotatedVideoWriter( ::std::string p_output_path, double p_frames_per_second );

    /// @brief Non copiable : possède un cv::VideoWriter ouvert sur un fichier.
    AnnotatedVideoWriter( const AnnotatedVideoWriter& p_other ) = delete;

    /// @brief Non copiable : possède un cv::VideoWriter ouvert sur un fichier.
    AnnotatedVideoWriter& operator=( const AnnotatedVideoWriter& p_other ) = delete;

    /// @brief Ferme proprement le fichier.
    ~AnnotatedVideoWriter() override;

    /// @brief Écrit la frame annotée, en ouvrant le fichier au premier appel.
    /// @param p_frame_index     Index de la frame (inutilisé ici).
    /// @param p_model           Modèle de voie (inutilisé ici).
    /// @param p_annotated_frame Image annotée à écrire.
    /// @param p_elapsed_ms      Durée de traitement (inutilisée ici).
    void on_frame( int p_frame_index,
                   const LaneModel& p_model,
                   const ::cv::Mat& p_annotated_frame,
                   double p_elapsed_ms ) override;

    /// @brief Indique si l'ouverture a échoué ou si une frame vide a été reçue.
    /// @return true en cas d'échec.
    bool has_failed() const;

    /// @brief Indique si l'écriture est dans un état d'échec définitif.
    /// @return has_failed().
    bool has_fatal_error() const override { return has_failed(); }

    /// @brief L'écriture vidéo exploite l'image annotée.
    /// @return true.
    bool needs_annotated_frame() const override { return true; }

  private:
    ::std::string m_output_path;      ///< Chemin du fichier vidéo.
    double m_frames_per_second;       ///< Cadence déclarée.
    ::cv::VideoWriter m_video_writer;  ///< Writer OpenCV sous-jacent.
    bool m_has_failed;                ///< true en cas d'échec.
  };
