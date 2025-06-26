#pragma once
#include "TransitionModel.hpp"

class MazeModel : public TransistionModel {
public:
    const int width = 6,height = 10;   
    MazeModel();

    MapState init() override;
    MapState result(const MapState& state, Action action) override;
    std::array<std::pair<Action, float>, 4> action(const MapState& state) override;
    bool isGoalState(const MapState& state) override;
};