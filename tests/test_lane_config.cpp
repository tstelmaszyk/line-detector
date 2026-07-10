#include "doctest.h"
#include "LaneConfig.h"

TEST_CASE("LaneConfig fournit des valeurs par defaut saines") {
    LaneConfig c;
    CHECK(c.windowCount > 0);
    CHECK(c.windowMargin > 0);
    CHECK(c.windowMinPix > 0);
    CHECK(c.whiteThreshold > 0);
    CHECK(c.srcTopWidthRatio > 0.0f);
    CHECK(c.srcTopYRatio > 0.0f);
    CHECK(c.srcTopYRatio < 1.0f);
    CHECK(c.defaultLaneWidthPx == doctest::Approx(0.0)); // 0 = pas de reconstruction par defaut
}
