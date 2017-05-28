#include "players.h"
#include "log.h"
#include "astar.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <sstream>
#include <random>

Level::Level() : _vbuffer(_shader), _tiles(nullptr), width(256), height(256) { }

Level::~Level() { }

LevelTileTypes Level::tile(int x, int y) const
{
    if (x < 0 || x >= this->width || y < 0 || y > this->height) return LevelTileTypes::NonWalkable;

    auto tile = this->_tiles[y * this->width + x];

    if (tile.rgba[3] == 0) return LevelTileTypes::NonWalkable;
    if (tile.rgba[0] == 0 && tile.rgba[1] == 255 && tile.rgba[2] == 0) return LevelTileTypes::CounterTerroristSpawn;
    if (tile.rgba[0] == 255 && tile.rgba[1] == 0 && tile.rgba[2] == 0) return LevelTileTypes::TerroristSpawn;
    if (tile.rgba[0] == 255 && tile.rgba[1] == 255 && tile.rgba[2] == 0) return LevelTileTypes::NonWalkableButSeeThrough;

    return LevelTileTypes::Walkable;
}

void Level::load(const std::string& level)
{
    this->_shader.compileFromFile("shaders/gl3/vertex.glsl", "shaders/gl3/fragment.glsl");

    std::stringstream ss;
    ss << "radars/" << level << ".png";
    this->_level.setup();
    this->_level.load(ss.str());

    auto x = this->_level.width();
    auto y = this->_level.height();
    this->_vbuffer
            << PlayerVertex({ {    x, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } })
            << PlayerVertex({ {    x,    y, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } })
            << PlayerVertex({ { 0.0f,    y, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } })
            << PlayerVertex({ { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
    this->_vbuffer.setup();

    std::stringstream sss;
    sss << "radars/" << level << "-walkable.png";

    int comp;
    this->_tiles = (Tile*)stbi_load(sss.str().c_str(), &this->width, & this->height, &comp, 4);
    for (int y = 0; y < this->height; y++)
    {
        for (int x = 0; x < this->width; x++)
        {
            auto t = this->tile(x, y);
            if (t == LevelTileTypes::CounterTerroristSpawn)
            {
                // Counter Terrorist spawn
                Player::Manager().addPlayer(x, y, Teams::CounterTerrorist);
            }
            else if (t == LevelTileTypes::TerroristSpawn)
            {
                // Terrorist spawn
                Player::Manager().addPlayer(x, y, Teams::Terrorist);
            }
        }
    }
}

void Level::render(const glm::mat4& proj, const glm::mat4& view)
{
    this->_shader.use();
    this->_shader.setupMatrices(glm::value_ptr(proj),
                                glm::value_ptr(view),
                                glm::value_ptr(glm::mat4(1.0f))
                                );
    this->_level.use();
    this->_vbuffer.render();
}

Player::Player() : _team(Teams::Teamless), _health(1.0f), _dir(0.0f, -1.0f, 0.0f) { }

Player::~Player() { }

Bullet::Bullet(const Player* gunner) : _deleted(false), _gunner(gunner), _pos(gunner->_pos), _dir(gunner->_dir), _weight(0.2f) { }

Bullet::~Bullet() { }

PlayerManager& Player::Manager()
{
    if (PlayerManager::_instance == nullptr) PlayerManager::_instance = new PlayerManager();

    return *PlayerManager::_instance;
}

PlayerManager* PlayerManager::_instance = nullptr;

PlayerManager::PlayerManager() : _buffer(_shader), _selectedPlayer(nullptr) { }

PlayerManager::~PlayerManager() { }

void PlayerManager::resetPlayers()
{
    while (!this->_players.empty())
    {
        auto player = *this->_players.begin();
        this->_players.erase(this->_players.begin());
        delete player;
    }
    this->_selectedPlayer = nullptr;
}

static float playerScale = 8.0f;

void PlayerManager::setup()
{
    this->_playerTexture.setup();
    this->_playerTexture.load("player.png");

    this->_selectedPlayerTexture.setup();
    this->_selectedPlayerTexture.load("selected-player.png");

    this->_deadPlayerTexture.setup();
    this->_deadPlayerTexture.load("dead-player.png");

    this->_bulletTexture.setup();
    this->_bulletTexture.load("bullet.png");

    this->_shader.compileFromFile("shaders/gl3/vertex.glsl", "shaders/gl3/fragment.glsl");

    this->_buffer
            << PlayerVertex({ {  playerScale, -playerScale, 0.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 0.0f } })
            << PlayerVertex({ {  playerScale,  playerScale, 0.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 1.0f } })
            << PlayerVertex({ { -playerScale,  playerScale, 0.0f }, { 0.0f, 1.0f, 0.0f }, {  0.0f, 1.0f } })
            << PlayerVertex({ { -playerScale, -playerScale, 0.0f }, { 0.0f, 1.0f, 0.0f }, {  0.0f, 0.0f } });
    this->_buffer.setup();
}

void PlayerManager::update(float diff)
{
    float speed = 50.0f;
    float distanceInThisTick = speed * diff;

    for (Player* player : this->_players)
    {
        if (player->_health <= 0.0f) continue;

        auto todo = player->_walkTo - player->_pos;
        if (glm::length(todo) < distanceInThisTick)
        {
            player->_pos = player->_walkTo;
            if (!player->_path.empty())
            {
                auto to = player->_path.front();
                player->_walkTo = glm::vec3(to.x * playerScale, to.y * playerScale, 0.0f);
                player->_path.pop();
            }
        }
        else if (glm::length(todo) > 0.001f)
        {
            player->_pos += glm::normalize(todo) * distanceInThisTick;
        }

        auto dir = glm::normalize(player->_pos - player->_walkTo);
        if (glm::length(dir) > 0.001f) player->_dir = dir;
    }

    float bulletSpeed = 400.0f;
    for (auto bullet : this->_bullets)
    {
        if (bullet->_deleted) continue;

        bullet->_pos += (bullet->_dir * bulletSpeed * diff * -1.0f);
        auto type = this->_level.tile(int(bullet->_pos.x / playerScale), int(bullet->_pos.y / playerScale));
        bullet->_deleted = (type == LevelTileTypes::NonWalkable);

        if (!bullet->_deleted)
        {
            for (Player* player : this->_players)
            {
                if (player == bullet->_gunner) continue;
                if (player->_health <= 0.0f) continue;
                if (glm::length(player->_pos - bullet->_pos) < playerScale)
                {
                    player->_health -= bullet->_weight;
                    bullet->_deleted = true;
                    break;
                }
            }
        }
    }
}

void PlayerManager::render(const glm::mat4 &proj, const glm::mat4 &view)
{
    this->_level.render(proj, view);

    this->_playerTexture.use();
    for (auto player : this->_players)
    {
        if (player->_health <= 0.0f) continue;

        auto model = glm::translate(glm::mat4(1.0f), player->_pos);
        if (glm::length(player->_dir) > 0.001f)
        {
            model = glm::rotate(model, std::atan2(player->_dir.x, player->_dir.y), glm::vec3(0.0f, 0.0f, -1.0f));
        }
        this->_shader.setupMatrices(glm::value_ptr(proj),
                                    glm::value_ptr(view),
                                    glm::value_ptr(model)
                                    );
        this->_buffer.render();
    }

    this->_bulletTexture.use();
    for (auto bullet : this->_bullets)
    {
        if (bullet->_deleted) continue;

        auto model = glm::translate(glm::mat4(1.0f), bullet->_pos);
        if (glm::length(bullet->_dir) > 0.001f)
        {
            model = glm::rotate(model, std::atan2(bullet->_dir.x, bullet->_dir.y), glm::vec3(0.0f, 0.0f, -1.0f));
        }
        this->_shader.setupMatrices(glm::value_ptr(proj),
                                    glm::value_ptr(view),
                                    glm::value_ptr(model)
                                    );
        this->_buffer.render();
    }

    if (this->_selectedPlayer != nullptr && this->_selectedPlayer->_health > 0.0f)
    {
        this->_selectedPlayerTexture.use();
        this->_shader.setupMatrices(glm::value_ptr(proj),
                                    glm::value_ptr(view),
                                    glm::value_ptr(glm::translate(glm::mat4(1.0f), this->_selectedPlayer->_pos))
                                    );
        this->_buffer.render();
    }

    this->_deadPlayerTexture.use();
    for (auto player : this->_players)
    {
        if (player->_health > 0.0f) continue;

        this->_shader.setupMatrices(glm::value_ptr(proj),
                                    glm::value_ptr(view),
                                    glm::value_ptr(glm::translate(glm::mat4(1.0f), player->_pos))
                                    );
        this->_buffer.render();
    }
}

Player* PlayerManager::addPlayer(int x, int y, Teams team)
{
    static std::default_random_engine generator;
    static std::uniform_int_distribution<int> distribution(0, PLAYER_NAME_COUNT);

    auto player = new Player();

    player->_name = playerNames[distribution(generator)];
    player->_pos = player->_walkTo = PlayerManager::levelToWorldLocation(x, y);
    player->_team = team;

    this->_players.insert(player);
    return player;
}

void PlayerManager::selectPlayer(Player* player)
{
    this->_selectedPlayer = player;
}

void PlayerManager::clickAt(int x, int y)
{
    std::set<Player*> selection;

    for (Player* player : this->_players)
    {
        if (glm::length(player->_pos - glm::vec3(x, y, 0)) < playerScale) selection.insert(player);
    }

    if (selection.size() > 0)
    {
        auto found = selection.find(this->_selectedPlayer);

        this->_selectedPlayer = *(found != selection.end() && ++found != selection.end() ? found : selection.begin());
        std::cout << this->_selectedPlayer->_name << std::endl;
    }
    else if (this->_selectedPlayer != nullptr)
    {
        auto target = this->_level.tile(x / playerScale, y / playerScale);
        if (target == LevelTileTypes::Walkable)
        {
            tPosition to = { int(x / playerScale), int(y / playerScale) };
            tPosition from = { int(this->_selectedPlayer->_pos.x / playerScale), int(this->_selectedPlayer->_pos.y / playerScale) };
            this->_selectedPlayer->_path = obj_GetAStarPath(from, to, [this] (const tPosition& position) {
                auto type = this->_level.tile(position.x, position.y);
                return (type == LevelTileTypes::Walkable) ||
                        (type == LevelTileTypes::CounterTerroristSpawn) ||
                        (type == LevelTileTypes::TerroristSpawn);
            });
        }
    }
}

void PlayerManager::shoot()
{
    if (this->_selectedPlayer != nullptr)
    {
        for (auto bullet : this->_bullets)
        {
            if (bullet->_deleted)
            {
                bullet->_deleted = false;
                bullet->_dir = this->_selectedPlayer->_dir;
                bullet->_pos = this->_selectedPlayer->_pos;
                return;
            }
        }

        // When we reach this point in code, no available bullet is found, and we are making a new one
        this->_bullets.insert(new Bullet(this->_selectedPlayer));
    }
}

glm::vec3 PlayerManager::levelToWorldLocation(int x, int y)
{
    return glm::vec3(x * playerScale, y * playerScale, 0.0f);
}

std::string PlayerManager::playerNames[PLAYER_NAME_COUNT] = {
    "Albert",
    "Allen",
    "Bert",
    "Bob",
    "Cecil",
    "Clarence",
    "Elliot",
    "Elmer",
    "Ernie",
    "Eugene",
    "Fergus",
    "Ferris",
    "Frank",
    "Frasier",
    "Fred",
    "George",
    "Graham",
    "Harvey",
    "Irwin",
    "Larry",
    "Lester",
    "Marvin",
    "Neil",
    "Niles",
    "Oliver",
    "Opie",
    "Ryan",
    "Toby",
    "Ulric",
    "Ulysses",
    "Uri",
    "Waldo"
};
