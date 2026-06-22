#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "DetectLines.h"
#include "RegionOfInterest.h"
#include "VideoCaracteristics.h"

#include <iostream>
#include <string>

using namespace cv;


int main(int argc, char** argv )
{
    // Chemins en arguments (pratique en conteneur) ; valeurs par defaut sinon.
    //   argv[1] = image d'entree, argv[2] = image de sortie.
    std::string input_path  = (argc > 1) ? argv[1] : "img_piste/img2.jpg";
    std::string output_path = (argc > 2) ? argv[2] : "output.jpg";

    cv::Mat image = imread(input_path, IMREAD_COLOR);
    if ( !image.data )
    {
        std::cout << "Impossible de lire l'image : " << input_path << std::endl;
        return -1;
    }

    cv::Mat image_out;
    VideoCaracteristics video_properties (image);
    DetectLines detecteur(video_properties);
    detecteur.draw_lines(image, image_out);

    // Pas de fenetre GUI dans un conteneur : on ecrit le resultat sur disque.
    if ( !imwrite(output_path, image_out) )
    {
        std::cout << "Impossible d'ecrire l'image de sortie : " << output_path << std::endl;
        return -1;
    }
    std::cout << "Image traitee ecrite dans : " << output_path << std::endl;
    return 0;
}