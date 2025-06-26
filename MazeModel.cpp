#include <map>
#include "MazeModel.hpp"

void genPatternState(MapState* state) 
{
    for(int i = 0; i<state->size; i+=4) 
    {
        state->objects[i] = Object::stone;
    }
    state->goalPos = sf::Vector2i(state->width-1, state->height-1);
}

bool isStatePossible(const MapState& stateIn) {
    auto resultMove = stateIn.playerPos;
    auto resultObj = stateIn.getObject(resultMove);
    if (resultObj == Object::stone || resultObj == Object::oob) return false;
    return true;
}

bool isStatePossible(const MapState& stateIn, sf::Vector2i customPlayerCord) {
    auto resultObj = stateIn.getObject(customPlayerCord);
    if (resultObj == Object::stone || resultObj == Object::oob) return false;
    return true;
}



MapState MazeModel::init()
{
    auto state = MapState(width,height);
    genPatternState(&state);

    return state;
}

MazeModel::MazeModel()
{
}

MapState MazeModel::result(const MapState& stateIn, Action action)
{
    sf::Vector2i deltaMove;
    if(action == Action::MoveLeft) deltaMove = sf::Vector2i(-1,0);
    if(action == Action::MoveRight) deltaMove = sf::Vector2i(1,0);
    if(action == Action::MoveUp) deltaMove = sf::Vector2i(0,-1);
    if(action == Action::MoveDown) deltaMove = sf::Vector2i(0,1);
    
    //Quick Check if the action is possible "without calling isStatepossible Function"
    sf::Vector2i destCords = stateIn.playerPos + deltaMove;
    if(!isStatePossible(stateIn, destCords)) return stateIn;

    //Move
    MapState stateOut = stateIn;
    stateOut.playerPos += deltaMove;
    return stateOut;

}

float getPathPriority(const MapState &stateIn, sf::Vector2i cord) {
    auto delta =  stateIn.goalPos - cord;
    float dist = delta.x*delta.x+delta.y*delta.y;
    return dist;
}

bool compare(std::pair<Action, float> one,std::pair<Action, float> two) {
    return one.second > two.second;
}

std::array<std::pair<Action, float>, 4> MazeModel::action(const MapState &stateIn)
{
    std::array<std::pair<Action, float>, 4> actions = {std::pair<Action, float>(Action::Null,0.0f), std::pair<Action, float>(Action::Null,0.0f), std::pair<Action, float>(Action::Null,0.0f), std::pair<Action, float>(Action::Null,0.0f)};
    if (isStatePossible(stateIn, stateIn.playerPos + sf::Vector2i(-1,0))) {
        actions[0].first = Action::MoveLeft; 
        actions[0].second = getPathPriority(stateIn, stateIn.playerPos + sf::Vector2i(-1,0));
    }
    if (isStatePossible(stateIn, stateIn.playerPos + sf::Vector2i(1,0))) { 
        actions[1].first = Action::MoveRight;
        actions[1].second = getPathPriority(stateIn, stateIn.playerPos + sf::Vector2i(1,0));
    }
    if (isStatePossible(stateIn, stateIn.playerPos + sf::Vector2i(0,-1))) { 
        actions[2].first = Action::MoveUp;
        actions[2].second = getPathPriority(stateIn, stateIn.playerPos + sf::Vector2i(0,-1));
    }
    if (isStatePossible(stateIn, stateIn.playerPos + sf::Vector2i(0,1))) { 
        actions[3].first = Action::MoveDown;
        actions[3].second = getPathPriority(stateIn, stateIn.playerPos + sf::Vector2i(0,1));
    }
    std::sort(actions.begin(), actions.end(), compare);
    return actions;
}

bool MazeModel::isGoalState(const MapState &state)
{
    return state.playerPos == state.goalPos;
}
