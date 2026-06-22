#pragma once

#include <string>
#include <opencv2/core.hpp>

/*!
*  \brief Destination d'ecriture d'une image (resultat final ou trace de debug).
*
*  `name` est le nom de fichier complet (ex. "output.jpg", "debug_01_gray.jpg").
*  Aucun prefixe ni extension n'est impose par l'interface : le meme sink sert
*  au resultat comme aux traces. `save` renvoie true si l'image a ete ecrite.
*/
class ImageSink
{
    public:
        virtual ~ImageSink() = default;
        virtual bool save(const std::string& name, const cv::Mat& frame) = 0;
};
