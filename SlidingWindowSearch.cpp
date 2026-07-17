#include "SlidingWindowSearch.h"
#include "SmartAssert.h"

#include <algorithm>
#include <opencv2/imgproc.hpp>

SlidingWindowSearch::SlidingWindowSearch(const VideoCaracteristics& video, const LaneConfig& config, ImageSink& debug_sink)
    : video_properties(video), config(config), debug_sink(debug_sink)
{
}

LanePixels SlidingWindowSearch::search(const cv::Mat& bev) const
{
    SMART_ASSERT(!bev.empty(), "search: BEV vide");
    SMART_ASSERT(bev.type() == CV_8UC1, "search: attend un binaire mono-canal");

    const int H = bev.rows;
    const int W = bev.cols;

    // Histogramme des colonnes sur une bande basse etroite : la base doit refleter
    // la position des lignes au TOUT-BAS (ou commence la 1re fenetre). Sur une moitie
    // basse, une ligne fortement courbee balaie horizontalement et l'argmax tombe
    // au-dessus du bas reel, decalant la 1re fenetre a cote des pixels du ras du bas.
    const int histTop = std::max(0, H - static_cast<int>(config.histogramBandRatio * H));
    std::vector<int> hist(W, 0);
    for (int y = histTop; y < H; ++y) {
        const uchar* row = bev.ptr<uchar>(y);
        for (int x = 0; x < W; ++x) if (row[x] > 0) hist[x]++;
    }

    // Deux pics : gauche dans [0, W/2), droite dans [W/2, W).
    int leftBase = 0, rightBase = W / 2;
    int leftMax = -1, rightMax = -1;
    for (int x = 0; x < W / 2; ++x) if (hist[x] > leftMax)  { leftMax = hist[x];  leftBase = x; }
    for (int x = W / 2; x < W; ++x) if (hist[x] > rightMax) { rightMax = hist[x]; rightBase = x; }

    LanePixels pixels;
    const int nWindows = config.windowCount;
    const int margin   = config.windowMargin;
    const int minPix   = config.windowMinPix;
    const int winH     = std::max(1, H / nWindows); // H%nWindows lignes du haut non scannees si H non divisible (sans consequence a 720p)

    int leftCur = leftBase, rightCur = rightBase;

    for (int w = 0; w < nWindows; ++w) {
        const int yLow  = std::max(0, H - (w + 1) * winH);
        const int yHigh = H - w * winH;
        const int m     = (w == 0) ? config.firstWindowMargin : margin; // 1re fenetre elargie : rattrape la queue qui part vite en courbe au ras du bas

        // Fenetre gauche.
        int sumX = 0, count = 0;
        for (int y = yLow; y < yHigh; ++y) {
            const uchar* row = bev.ptr<uchar>(y);
            for (int x = std::max(0, leftCur - m); x < std::min(W, leftCur + m); ++x)
                if (row[x] > 0) { pixels.left.emplace_back(x, y); sumX += x; count++; }
        }
        if (count > minPix) leftCur = sumX / count; // sinon on ne bouge pas

        // Fenetre droite.
        sumX = 0; count = 0;
        for (int y = yLow; y < yHigh; ++y) {
            const uchar* row = bev.ptr<uchar>(y);
            for (int x = std::max(0, rightCur - m); x < std::min(W, rightCur + m); ++x)
                if (row[x] > 0) { pixels.right.emplace_back(x, y); sumX += x; count++; }
        }
        if (count > minPix) rightCur = sumX / count;
    }

    // Trace debug : pixels gauche en rouge, droite en bleu.
    cv::Mat dbg;
    cv::cvtColor(bev, dbg, cv::COLOR_GRAY2BGR);
    for (const auto& p : pixels.left)  dbg.at<cv::Vec3b>(p) = cv::Vec3b(0, 0, 255);
    for (const auto& p : pixels.right) dbg.at<cv::Vec3b>(p) = cv::Vec3b(255, 0, 0);
    debug_sink.save("debug_03_windows.jpg", dbg);

    return pixels;
}
