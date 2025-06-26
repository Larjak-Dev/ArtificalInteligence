#include "TransitionModel.hpp"
#include <cstring>

TransistionModel::TransistionModel()
{
}

MapState::MapState() : init(false)
{
}

MapState::MapState(int width, int height) : init(true), width(width), height(height), objects(width * height, Object::air), playerPos(0, 0), size(width * height)
{

}

Object MapState::getObject(sf::Vector2i cord) const
{
    //Out of bounds
    if(cord.x>this->width-1 || cord.x<0 || cord.y>this->height-1 || cord.y<0) return Object::oob;

    return this->objects[cord.y*this->width+cord.x];
}

bool MapState::setObject(sf::Vector2i cord, Object obj)
{
    if(cord.x>this->width-1 || cord.x<0 || cord.y>this->height-1 || cord.y<0) return false;
    this->objects[cord.y*this->width+cord.x] = obj;
    return true;
}

bool MapState::compare(const MapState& compState) const
{
    if(this->playerPos != compState.playerPos) return false;
    return (this->objects.size() == compState.objects.size() && std::memcmp(this->objects.data(), compState.objects.data(), this->objects.size()) == 0);

    //return this->objects == compState.objects;
}