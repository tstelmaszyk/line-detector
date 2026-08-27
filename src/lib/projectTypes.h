#pragma once

/// @file
/// @brief Types de dimensions image partagés dans le projet.

#include <cstdint>

/// @brief Dimension image en pixels (largeur ou hauteur).
typedef ::std::uint16_t DimensionImage;

/// @brief Index d'une frame dans un flux (0 pour la première).
typedef ::std::int32_t FrameIndex;
