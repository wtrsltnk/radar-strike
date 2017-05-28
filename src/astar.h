#ifndef ASTAR_H
#define ASTAR_H

#include <functional>
#include <queue>

typedef struct sPosition
{
    int x, y;
    bool operator < (const sPosition & other) const
    {
        if (x < other.x) return true;
        if (x == other.x && y < other.y) return true;
        return false;
    }
    bool operator == (const sPosition & other) const
    {
        return (*this) < other == other < (*this);
    }
    struct sPosition &setX(float x)
    {
        this->x = x;
        return *this;
    }
    struct sPosition &setY(float y)
    {
        this->y = y;
        return *this;
    }
} tPosition;

std::queue<tPosition> obj_GetAStarPath(const tPosition & from, const tPosition & to, std::function<bool (const tPosition&)> isWalkable);

#endif // ASTAR_H
