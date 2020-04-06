#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.h"
#include "scanner.h"

TEST_CASE( "Factorials are computed", "[factorial]" ) {
    REQUIRE( 1 == 1 );

	testFunction(1,2);
}
