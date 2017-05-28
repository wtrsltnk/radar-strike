#include "catch.hpp"
#include "players.h"

TEST_CASE("Select one player", "[players]" )
{
    Player::Manager().resetPlayers();
    REQUIRE(Player::Manager()._selectedPlayer == nullptr);

    auto a = Player::Manager().addPlayer(0, 0, Teams::CounterTerrorist);

    // Clicking far away from the player, will not select it
    Player::Manager().clickAt(0, 100);
    REQUIRE(Player::Manager()._selectedPlayer == nullptr);

    // Clicking nearby the player will select it
    Player::Manager().clickAt(0, 1);
    REQUIRE(Player::Manager()._selectedPlayer == a);
}

TEST_CASE("Circle select two players", "[players]" )
{
    Player::Manager().resetPlayers();
    REQUIRE(Player::Manager()._selectedPlayer == nullptr);

    auto a = Player::Manager().addPlayer(0, 0, Teams::CounterTerrorist);
    auto b = Player::Manager().addPlayer(0, 1, Teams::CounterTerrorist);

    Player::Manager().clickAt(0, 1);
    auto selection1 = Player::Manager()._selectedPlayer;

    Player::Manager().clickAt(0, 1);
    auto selection2 = Player::Manager()._selectedPlayer;

    Player::Manager().clickAt(0, 1);
    auto selection3 = Player::Manager()._selectedPlayer;

    REQUIRE(selection1 != selection2);
    REQUIRE(selection2 != selection3);
    REQUIRE(selection1 == selection3);
}

TEST_CASE("Circle select three players", "[players]" )
{
    Player::Manager().resetPlayers();
    REQUIRE(Player::Manager()._selectedPlayer == nullptr);

    auto a = Player::Manager().addPlayer(0, 0, Teams::CounterTerrorist);
    auto b = Player::Manager().addPlayer(0, 1, Teams::CounterTerrorist);
    auto c = Player::Manager().addPlayer(0, 1, Teams::CounterTerrorist);

    Player::Manager().clickAt(0, 1);
    auto selection1 = Player::Manager()._selectedPlayer;

    Player::Manager().clickAt(0, 1);
    auto selection2 = Player::Manager()._selectedPlayer;

    REQUIRE(selection1 != selection2);

    Player::Manager().clickAt(0, 1);
    auto selection3 = Player::Manager()._selectedPlayer;

    REQUIRE(selection2 != selection3);

    Player::Manager().clickAt(0, 1);
    auto selection4 = Player::Manager()._selectedPlayer;

    REQUIRE(selection3 != selection4);
    REQUIRE(selection1 == selection4);

    Player::Manager().clickAt(0, 1);
    auto selection5 = Player::Manager()._selectedPlayer;

    REQUIRE(selection4 != selection5);
    REQUIRE(selection2 == selection5);
}
