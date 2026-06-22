#pragma once

#include "ImageSink.h"

/*!
*  \brief Sink no-op : n'ecrit rien. Defaut quand le debug est desactive.
*  Renvoie true (rien a ecrire n'est pas un echec).
*/
class NullImageSink : public ImageSink
{
    public:
        bool save(const std::string&, const cv::Mat&) override { return true; }
};
