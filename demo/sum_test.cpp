#include "sum.cpp"

TEST_CASE( "Pair of numbers are summed", "[sum]" ) {
	REQUIRE( sum(1,2) == 3 );
	REQUIRE( sum(2,4) == 6 );
	REQUIRE( sum(1,4) == 5 );
}
