#include "doctest.h"

#include <string>

#include "SmartAssert/SmartAssert.h"

TEST_CASE( "smart_assert::format contient fichier, ligne, condition et message" )
{
  const ::std::string message =
    ::smart_assert::format( "Foo.cpp", 42, "x > 0", "x doit etre positif" );

  CHECK( ::std::string::npos != message.find( "Foo.cpp" ) );
  CHECK( ::std::string::npos != message.find( "42" ) );
  CHECK( ::std::string::npos != message.find( "x > 0" ) );
  CHECK( ::std::string::npos != message.find( "x doit etre positif" ) );
}

TEST_CASE( "SMART_ASSERT ne fait rien quand la condition est vraie" )
{
  int reached = 0;

  SMART_ASSERT( 1 + 1 == 2, "arithmetique cassee" );
  reached = 1;

  CHECK( 1 == reached );
}

TEST_CASE( "EXIT_IF_FAILED ne fait rien quand la condition est vraie" )
{
  int reached = 0;

  EXIT_IF_FAILED( 1 + 1 == 2, "alea d'environnement inattendu" );
  reached = 1;

  CHECK( 1 == reached );
}
