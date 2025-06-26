#include "SearchAlgorithm.hpp"

void SearchAlgorithm::addNode(const Node &prevNode, Action action)
{
    if(action == Action::Null) return;

    Node node;
    node.init = true;
    node.mapState = this->tm->result(prevNode.mapState, action);

    node.actionPath = prevNode.actionPath;
    node.actionPath.push_back(action);
    node.totalPathCost = prevNode.totalPathCost + 1;
    node.totalActionCount = prevNode.totalActionCount + 1;
    this->pushFrontierNode(node);
}

void SearchAlgorithm::initNode(const MapState &map)
{
    this->initStateObj = map;

    Node node;
    node.init = true;
    node.mapState = map;
    node.totalPathCost = 0;
    node.totalActionCount = 0;

    this->pushFrontierNode(node);
}

Node SearchAlgorithm::getFrontierNode()
{
    return frontier.front();
}

void SearchAlgorithm::popFrontierNode()
{
    frontier.pop();
}

void SearchAlgorithm::pushFrontierNode(Node node)
{
    this->frontier.push(node);
}

SearchAlgorithm::SearchAlgorithm()
{
}

SearchAlgorithm::SearchAlgorithm(const MapState &initMapstate, TransistionModel *tm) : tm(tm)
{
    initNode(initMapstate);
}

SearchState SearchAlgorithm::searchDeepTick()
{
    if(frontier.empty()) {
        if (this->goalNode.init == true) return SearchState::Found;
        if (this->goalNode.init == false) return SearchState::Failure;
    }
    
    //Pick a Node
    const Node searchNode = this->getFrontierNode();

    //Check if Node has GoalState
    bool isGoal = tm->isGoalState(searchNode.mapState);
    if(isGoal) 
    {
        //Repleaces the Previuos GoalNode if the Found GoalNode has lower PathCost than Previuos GoalNode.
        if(searchNode.totalPathCost < this->goalNode.totalPathCost || this->goalNode.init == false) this->goalNode = searchNode;
    }
    this->popFrontierNode();
    if(!isGoal) {
        //Checks if PickedNode has already been explored
        bool isExplored = false;
        for(const MapState& mstate : this->exploredStates) 
        {
            if(searchNode.mapState.compare(mstate)) isExplored = true;
        }
    

        if(isExplored) 
        { 
            ///This code will try to compare the PickedNode with nodes along an already found GoalPath.
            ///If the PickedNode has a more efficient PathCost to an Node along the GoalPath then it will 
            ///replace the starting Actions of the Path to the more efficient Actions from PickedNode.
            
            //Get the corrosponding node with the same MapState as PickedNode and correct amount of PathCost value. 
            auto nodeFromGoalPath = this->getNodeWithinActionPath(this->goalNode, searchNode.mapState);
            if(nodeFromGoalPath.init == true && nodeFromGoalPath.totalPathCost > searchNode.totalPathCost) {
                ///Replace goal path with the new quicker found path
                //Set new goal path cost
                int deltaPathCost = searchNode.totalPathCost - nodeFromGoalPath.totalPathCost;
                this->goalNode.totalPathCost += deltaPathCost;

                int deltaActionCount = searchNode.totalActionCount - nodeFromGoalPath.totalActionCount;
                this->goalNode.totalActionCount += deltaActionCount;

                assert(goalNode.actionPath.size() + deltaActionCount > 0);
                std::vector<Action> newGoalActionData = std::vector<Action>(goalNode.actionPath.size() + deltaActionCount, Action());   //!UNSURE, POSSIBLE MISTAKES
                newGoalActionData.insert(newGoalActionData.begin(), searchNode.actionPath.begin(), searchNode.actionPath.end());
                newGoalActionData.insert(  
                    std::next(newGoalActionData.begin(), searchNode.totalActionCount), 
                    std::next(goalNode.actionPath.begin(), searchNode.totalActionCount), 
                    goalNode.actionPath.end()); 
                //!HOPEFULLY I WROTE THE INDECIES/Iterators RIGHT
                
                 
                goalNode.actionPath = newGoalActionData;
            }
            //Check if 
        } 
        else 
        {
            //Adds to explored
            this->exploredStates.push_back(searchNode.mapState);

            //Adds Action Nodes
            auto actions = tm->action(searchNode.mapState);
            for(auto action : actions) {
                auto mapState = searchNode.mapState;
                mapState = this->tm->result(mapState, action.first);
                
                if(!this->getNodeWithinActionPath(searchNode, mapState).init) {
                    this->addNode(searchNode, action.first);
                }
            }

        }
    }
    return SearchState::Searching;
}

SearchState SearchAlgorithm::searchTick()
{
    if(frontier.empty()) return SearchState::Failure;

    //Pick Back Node for searching
    const Node searchNode = getFrontierNode();

    //Check If Node has GoalState
    if(tm->isGoalState(searchNode.mapState)) 
    {
        this->goalNode = searchNode;
        return SearchState::Found;
    }

    //If not GoalState
    this->exploredStates.push_back(searchNode.mapState); //Add to already explored states
    this->popFrontierNode(); //Removes the searched node

    //Adds possible Nodes from Actions
    auto actions = tm->action(searchNode.mapState);
    for(const auto& action : actions) {
        this->addNode(searchNode, action.first);

        //Remove Action Node if State is already explored.
        for(const MapState& mstate : this->exploredStates) {
            if(this->getFrontierNode().mapState.compare(mstate)) this->popFrontierNode();
        }
    }
    return SearchState::Searching;
}

SearchState SearchAlgorithm::searchResult()
{
    SearchState searchState = SearchState::Searching;
    while(searchState == SearchState::Searching) {
        searchState = searchTick();
    }
    return searchState;
}

Node SearchAlgorithm::getResult()
{
   return this->goalNode; 
}

Node SearchAlgorithm::getNodeWithinActionPath(const Node &nodeWithpath, const MapState &mapState) const
{
    Node agent;
    agent.init = true;
    agent.mapState = this->initStateObj;
    agent.totalPathCost = 0;
    for(Action action : nodeWithpath.actionPath) {
        if (agent.mapState.compare(mapState)) return agent;
        agent.mapState = this->tm->result(agent.mapState, action);
        agent.totalPathCost += 1;
        agent.totalActionCount += 1;
    }
    return Node();
}
