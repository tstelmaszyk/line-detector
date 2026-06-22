#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "DetectLines.h"
#include "RegionOfInterest.h"
#include "VideoCaracteristics.h"
#include "ImageSink.h"
#include "DiskImageSink.h"
#include "NullImageSink.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

using namespace cv;


int main(int argc, char** argv )
{
    // argv[1] = image d'entree (defaut sinon). Le dossier de sortie est fixe
    // dans le code ; on n'encombre plus la ligne de commande avec un chemin de
    // sortie. Les traces de debug s'activent via LINE_DETECTOR_DEBUG (runtime).
    const std::string input_path = (argc > 1) ? argv[1] : "img_piste/img2.jpg";
    const std::string output_dir = "out";       // unique source de verite
    const std::string output_name = "output.jpg";

    cv::Mat image = imread(input_path, IMREAD_COLOR);
    if ( !image.data )
    {
        std::cout << "Impossible de lire l'image : " << input_path << std::endl;
        return -1;
    }

    // Resultat final : toujours ecrit sur disque.
    DiskImageSink result_sink(output_dir);

    // Traces du pipeline : disque si debug actif, sinon no-op.
    // Actif si LINE_DETECTOR_DEBUG est definie ET non vide.
    const char* debug_env = std::getenv("LINE_DETECTOR_DEBUG");
    std::unique_ptr<ImageSink> debug_sink;
    if (debug_env != nullptr && debug_env[0] != '\0')
    {
        debug_sink = std::make_unique<DiskImageSink>(output_dir);
    }
    else
    {
        debug_sink = std::make_unique<NullImageSink>();
    }

    cv::Mat image_out;
    VideoCaracteristics video_properties (image);
    DetectLines detecteur(video_properties, *debug_sink);
    detecteur.draw_lines(image, image_out);

    // Pas de fenetre GUI dans un conteneur : on ecrit le resultat sur disque.
    if ( !result_sink.save(output_name, image_out) )
    {
        std::cout << "Impossible d'ecrire l'image de sortie : " << output_dir << "/" << output_name << std::endl;
        return -1;
    }
    std::cout << "Image traitee ecrite dans : " << output_dir << "/" << output_name << std::endl;
    return 0;
}