#pragma once
#include <queue>
#include "TransitionModel.hpp"

struct Node {
    bool init = false;
    MapState mapState;
    int totalActionCount = 0;
    float totalPathCost = 0;
    std::vector<Action> actionPath;
};

enum class SearchState {
    Searching, Found, Failure, Done
};

class SearchAlgorithm {
    void addNode(const Node& prevNode, Action action);
    void initNode(const MapState& map);
public:
    MapState initStateObj;
    TransistionModel* tm;
    std::queue<Node> frontier;
    std::vector<MapState> exploredStates;
    Node goalNode;

    Node getFrontierNode();
    void popFrontierNode();
    void pushFrontierNode(Node node);

    SearchAlgorithm();
    SearchAlgorithm(const MapState& initMapstate, TransistionModel* tm);
    SearchState searchDeepTick();
    SearchState searchTick();
    SearchState searchResult();
    Node getResult();

    //Searches if 
    Node getNodeWithinActionPath(const Node& nodeWithpath, const MapState& mapState) const;

};