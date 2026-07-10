#include "doctest.h"
#include "SmartAssert.h"

TEST_CASE("smart_assert::format contient fichier, ligne, condition et message") {
    const std::string s = smart_assert::format("Foo.cpp", 42, "x > 0", "x doit etre positif");
    CHECK(s.find("Foo.cpp") != std::string::npos);
    CHECK(s.find("42") != std::string::npos);
    CHECK(s.find("x > 0") != std::string::npos);
    CHECK(s.find("x doit etre positif") != std::string::npos);
}

TEST_CASE("SMART_ASSERT ne fait rien quand la condition est vraie") {
    int reached = 0;
    SMART_ASSERT(1 + 1 == 2, "arithmetique cassee");
    reached = 1; // on n'arrive ici que si pas d'abort
    CHECK(reached == 1);
}
