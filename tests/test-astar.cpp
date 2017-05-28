#include "catch.hpp"

#include <astar.h>
#include <glm/glm.hpp>

TEST_CASE("All positions are walkable, but the start and destination are the same", "[astar]" ) {
    tPosition from = { 0, 0 };
    auto result = obj_GetAStarPath(from, from, [] (const tPosition& position) { return true; });
    REQUIRE(result.size() == 0);
}

TEST_CASE("There are no walkable positions", "[astar]" ) {
    tPosition from = { 0, 0 };
    tPosition to = { 10, 10 };
    auto result = obj_GetAStarPath(from, to, [] (const tPosition& position) { return false; });
    REQUIRE(result.size() == 0);
}

TEST_CASE("All positions are walkable", "[astar]" ) {
    tPosition from = { 0, 0 };
    tPosition to = { 10, 10 };
    auto result = obj_GetAStarPath(from, to, [] (const tPosition& position) { return true; });
    REQUIRE(result.size() == 10);
}

TEST_CASE("All positions in a straight line are walkable ", "[astar]" ) {
    tPosition from = { 0, 0 };
    tPosition to = { 10, 0 };
    auto result = obj_GetAStarPath(from, to, [] (const tPosition& position) { return true; });
    REQUIRE(result.size() == 10);
}

TEST_CASE("Two positions in a straight line are not /walkable ", "[astar]" ) {
    tPosition from = { 0, 0 };
    tPosition to = { 10, 0 };
    auto result = obj_GetAStarPath(from, to, [] (const tPosition& position) {
        if (position.y != 0) return true;
        return position.x != 4 && position.x != 5;
    });
    REQUIRE(result.size() == 10);
}
