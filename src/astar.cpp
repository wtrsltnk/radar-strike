#include "astar.h"
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>

class AStarNode
{
public:
    AStarNode(const tPosition & pos, AStarNode * p, const tPosition & goal);

    tPosition position;
    float f, g, h;
    AStarNode *prev;

    static float calculateG(AStarNode* previous = nullptr);
};

AStarNode::AStarNode(const tPosition& pos, AStarNode* previous, const tPosition& goal)
    : position(pos), prev(previous), g(10)
{
    this->g = AStarNode::calculateG(this->prev);

    // manhattan distance to the goal
    this->h = (abs(goal.x - this->position.x) + abs(goal.y - this->position.y)) * 10;
    this->f = this->g + this->h;
}

float AStarNode::calculateG(AStarNode* previous)
{
    return 10.0f + (previous != nullptr ? previous->g : 0.0f);
}

class queue_item
{
public:
    explicit queue_item(AStarNode* n);
    AStarNode *node;
    bool operator <(const queue_item & item) const;
};

queue_item::queue_item(AStarNode * n)
    : node(n)
{ }

bool queue_item::operator < (const queue_item & item) const
{
    return this->node->f < item.node->f;
}

std::queue<tPosition> obj_GetAStarPath(const tPosition & from, const tPosition & to, std::function<bool (const tPosition&)> isWalkable)
{
    std::queue<tPosition> res;

    // Are the start en destination the same?
    if ((from < to) == (to < from)) return res;

    // We are not going to find a path when the destination is not walkable
    if (!isWalkable(to)) return res;

    std::vector<queue_item> open;
    std::set<tPosition> closed;

    open.push_back(queue_item(new AStarNode(from, nullptr, to)));

    while (!open.empty())
    {
        auto current = open.begin()->node;
        open.erase(open.begin());

        // We found the finish
        if (current->position.x == to.x && current->position.y == to.y)
        {
            std::vector<tPosition> path;

            // Grab all nodes from the path we collected
            while (current->prev != nullptr)
            {
                path.push_back(current->position);
                current = current->prev;
            }

            // Reverse the order
            for (auto itr = path.rbegin(); itr != path.rend(); ++itr) res.push(*itr);

            // And return them
            return res;
        }

        // determine the positions of East, South, West and North
        tPosition eswn[8] {
            { current->position.x + 1, current->position.y },
            { current->position.x, current->position.y + 1 },
            { current->position.x - 1, current->position.y },
            { current->position.x, current->position.y - 1 },
            { current->position.x + 1, current->position.y + 1 },
            { current->position.x - 1, current->position.y + 1 },
            { current->position.x - 1, current->position.y - 1 },
            { current->position.x + 1, current->position.y - 1 }
        };

        for (int i = 0; i < 8; i++)
        {
            // If we already visited this location, we are not considering it again
            if (closed.find(eswn[i]) != closed.end()) continue;

            // If this position is not walkable, we are not considering it
            if (!isWalkable(eswn[i])) continue;

            auto found = std::find_if(open.begin(), open.end(), [eswn, i] (const queue_item& item) {
                return item.node->position == eswn[i];
            });

            if (found == open.end())
            {
                open.push_back(queue_item(new AStarNode(eswn[i], current, to)));
            }
            else if (found->node->g > AStarNode::calculateG(current))
            {
                open.erase(found);
                open.push_back(queue_item(new AStarNode(eswn[i], current, to)));
            }
        }

        closed.insert(current->position);
    }

    return res;
}
