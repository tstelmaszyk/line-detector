#pragma once

/// @file
/// @brief Statistiques d'une exécution du pipeline sur un flux.

/// @brief Compteurs d'une exécution, imprimés en fin de course.
struct RunStats
  {
  int frame_count = 0;          ///< Frames traitées.
  int detected_count = 0;       ///< Frames avec voie détectée.
  int reconstructed_count = 0;  ///< Frames avec un côté reconstruit.
  double compute_ms = 0.0;  ///< Durée cumulée du calcul (millisecondes).
  double render_ms = 0.0;   ///< Durée cumulée du rendu (millisecondes).
  };
