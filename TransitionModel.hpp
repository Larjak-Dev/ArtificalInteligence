#pragma once
#include <vector>
#include <array>
#include <SFML/Graphics.hpp>
#include <cassert>

enum class Object : uint8_t 
{
    oob, air, stone
};

class MapState
{
public:
    bool init;
    uint8_t width,height,size;

    MapState();
    MapState(int width, int height);
    sf::Vector2i playerPos;
    sf::Vector2i goalPos;
    std::vector<Object> objects;

    Object getObject(sf::Vector2i cord) const;
    bool setObject(sf::Vector2i cord, Object obj);

    bool compare(const MapState& mapstate) const;

};

constexpr int ActionCount = 4;

enum Action 
{
    Null, MoveLeft, MoveRight, MoveUp, MoveDown
};

class TransistionModel 
{
public:
    TransistionModel();

    virtual MapState init() = 0;
    virtual MapState result(const MapState& state, Action action) = 0;
    virtual std::array<std::pair<Action, float>, 4> action(const MapState& state) = 0;
    virtual bool isGoalState(const MapState& state) = 0;
    virtual ~TransistionModel() = default;
};