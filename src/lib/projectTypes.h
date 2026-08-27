#pragma once

/// @file
/// @brief Types de dimensions image partagés dans le projet.

#include <cstdint>

/// @brief Dimension image en pixels (largeur ou hauteur).
typedef ::std::uint16_t DimensionImage;

/// @brief Index d'une frame dans un flux (0 pour la première).
typedef ::std::int32_t FrameIndex;

/// @brief Durée exprimée en millisecondes (calcul, rendu).
typedef double DurationMs;

/// @brief Distance ou écart exprimé en pixels BEV (peut être négatif).
typedef double PixelOffset;

/// @brief Index d'un périphérique caméra (identifiant `cv::VideoCapture`).
typedef int CameraIndex;
