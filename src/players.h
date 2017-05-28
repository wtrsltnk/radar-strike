#ifndef PLAYERS_H
#define PLAYERS_H

#include <glm/glm.hpp>
#include <set>
#include <queue>

#include "astar.h"
#include "stb_image.h"
#include <gl.utilities.textures.h>
#include <gl.utilities.vertexbuffers.h>

#define PLAYER_NAME_COUNT 32

typedef Vertex<glm::vec3, glm::vec3, glm::vec2> PlayerVertex;
typedef Shader<glm::vec3, glm::vec3, glm::vec2> PlayerShader;
typedef VertexBuffer<glm::vec3, glm::vec3, glm::vec2> PlayerVertexBuffer;

typedef struct sTile
{
    unsigned char rgba[4];
} Tile;

enum class LevelTileTypes
{
    NonWalkable,
    Walkable,
    NonWalkableButSeeThrough,
    CounterTerroristSpawn,
    TerroristSpawn
};

class Level
{
public:
    Level();
    virtual ~Level();

    Texture _level;
    PlayerShader _shader;
    PlayerVertexBuffer _vbuffer;

    Tile* _tiles;
    int width;
    int height;

    void load(const std::string& level);
    LevelTileTypes tile(int x, int y) const;

    void render(const glm::mat4& proj, const glm::mat4& view);
};

enum class Teams
{
    Teamless,
    CounterTerrorist,
    Terrorist,
};

class Player
{
public:
    Player();
    virtual ~Player();

    Teams _team;
    std::string _name;
    float _health;

    glm::vec3 _dir;
    glm::vec3 _pos;
    glm::vec3 _walkTo;
    std::queue<tPosition> _path;

public:
    static class PlayerManager& Manager();
};

class Bullet
{
public:
    Bullet(const Player* gunner);
    virtual ~Bullet();

    const Player* _gunner;
    bool _deleted;
    glm::vec3 _pos;
    glm::vec3 _dir;
    float _weight;
};

class PlayerManager
{
    Texture _playerTexture;
    Texture _selectedPlayerTexture;
    Texture _deadPlayerTexture;
    Texture _bulletTexture;
    PlayerShader _shader;
    PlayerVertexBuffer _buffer;

    static std::string playerNames[PLAYER_NAME_COUNT];

    friend class Player;
    static PlayerManager* _instance;
public:
    PlayerManager();
    virtual ~PlayerManager();

    void resetPlayers();
    void setup();
    void update(float diff);
    void render(const glm::mat4& proj, const glm::mat4& view);

    Player* addPlayer(int x, int y, Teams team);
    void selectPlayer(Player* player);

    void clickAt(int x, int y);
    void shoot();

    static glm::vec3 levelToWorldLocation(int x, int y);
    static glm::vec3 worldToLevelLocation(int x, int y);

    std::set<Player*> _players;
    Player* _selectedPlayer;
    std::set<Bullet*> _bullets;

    Level _level;
};

#endif // PLAYERS_H
