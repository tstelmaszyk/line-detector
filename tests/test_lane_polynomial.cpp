#include "doctest.h"
#include "LanePolynomial.h"
#include <vector>
#include <cmath>

TEST_CASE("fit retrouve une parabole connue x = 2y^2 + 3y + 5") {
    std::vector<cv::Point> pts;
    for (int y = 0; y <= 100; y += 5) {
        const int x = static_cast<int>(std::lround(2.0*y*y + 3.0*y + 5.0));
        pts.emplace_back(x, y);
    }
    const LanePolynomial p = LanePolynomial::fit(pts);
    REQUIRE(p.valid);
    CHECK(p.a == doctest::Approx(2.0).epsilon(0.01));
    CHECK(p.b == doctest::Approx(3.0).epsilon(0.05));
    CHECK(p.c == doctest::Approx(5.0).epsilon(1.0));
}

TEST_CASE("fit renvoie invalid si trop peu de points") {
    const std::vector<cv::Point> pts = { {10,0}, {12,5} };
    const LanePolynomial p = LanePolynomial::fit(pts); // minPoints=3 par defaut
    CHECK_FALSE(p.valid);
}

TEST_CASE("evalAt evalue le polynome") {
    LanePolynomial p; p.a = 1; p.b = 0; p.c = 0; p.valid = true;
    CHECK(p.evalAt(3.0) == doctest::Approx(9.0));
}
