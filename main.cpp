#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "DetectLines.h"
#include "VideoCaracteristics.h"
#include "LaneConfig.h"
#include "ImageSink.h"
#include "DiskImageSink.h"
#include "NullImageSink.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char** argv)
{
    // argv[1] = image d'entree (defaut sinon). Dossier de sortie fixe.
    const std::string input_path  = (argc > 1) ? argv[1] : "img_piste/img2.jpg";
    const std::string output_dir  = "out";
    const std::string output_name = "output.jpg";

    cv::Mat image = cv::imread(input_path, cv::IMREAD_COLOR);
    if (!image.data) {
        std::cout << "Impossible de lire l'image : " << input_path << std::endl;
        return -1;
    }

    DiskImageSink result_sink(output_dir);

    // Traces de debug : disque si LINE_DETECTOR_DEBUG non vide, sinon no-op.
    const char* debug_env = std::getenv("LINE_DETECTOR_DEBUG");
    std::unique_ptr<ImageSink> debug_sink;
    if (debug_env != nullptr && debug_env[0] != '\0')
        debug_sink = std::make_unique<DiskImageSink>(output_dir);
    else
        debug_sink = std::make_unique<NullImageSink>();

    VideoCaracteristics video_properties(image);

    LaneConfig config;
    // Largeur de voie par defaut (pixels BEV) pour reconstruire un cote manquant.
    config.defaultLaneWidthPx = static_cast<double>(video_properties.width_pixel) * 0.5;

    cv::Mat image_out;
    DetectLines detecteur(video_properties, config, *debug_sink);
    const LaneModel model = detecteur.draw_lines(image, image_out);

    std::cout << "Voie detectee : " << (model.laneDetected ? "oui" : "non")
              << " | offset normalise : " << model.normalizedOffset
              << " | rayon : " << model.curvatureRadiusPx << " px" << std::endl;

    if (!result_sink.save(output_name, image_out)) {
        std::cout << "Impossible d'ecrire l'image de sortie : "
                  << output_dir << "/" << output_name << std::endl;
        return -1;
    }
    std::cout << "Image traitee ecrite dans : " << output_dir << "/" << output_name << std::endl;
    return 0;
}
