#include <list>
#include <memory>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

#include "SearchAlgorithm.hpp"
#include "MazeModel.hpp"

//App Configurable Constants
const int WindowWidth = 800;
const int WindowHeight = 600;
const sf::Color BackgroundColor = sf::Color::Black;

const sf::Color GridColor = sf::Color::White;
const int GridSize = 580;

//Variables
int windowHeight = WindowHeight;
int windowWidth = WindowWidth;
sf::Vector2f windowCenterCord = sf::Vector2f(WindowWidth/2, WindowHeight/2);
int gridSize = GridSize;


//Grid Variables
sf::VertexArray vbGrid;
int gridColumns = 10;
int gridRows = 10;
float gridRowSize;
float gridSquareSize;
float gridColumnSize;
sf::Vector2f gridTopLeftCord;

void generateGrid(const MapState& state) 
{
    gridRows = state.height;
    gridColumns = state.width;

    if(gridRows > gridColumns) {
        gridColumnSize = gridSize;
        gridSquareSize = gridColumnSize/gridRows;
        gridRowSize = gridSquareSize*gridColumns;
    } else {
        gridRowSize = gridSize;
        gridSquareSize = gridRowSize/gridColumns;
        gridColumnSize = gridSquareSize*gridRows;
    }

    
    gridTopLeftCord = windowCenterCord - sf::Vector2f(gridRowSize/2,gridColumnSize/2);


    vbGrid.setPrimitiveType(sf::PrimitiveType::Lines);

    sf::Vector2f topLeftPoint = gridTopLeftCord;
    int vertexCount = (gridColumns+1)*2 + (gridRows+1)*2;
    vbGrid.resize(vertexCount);

    for(int i = 0; i< gridColumns + 1; i++) 
    {
        
        vbGrid[i*2] = sf::Vertex(topLeftPoint + sf::Vector2f(gridSquareSize*i,0), GridColor);
        vbGrid[i*2+1] = sf::Vertex(topLeftPoint + sf::Vector2f(gridSquareSize*i,gridColumnSize), GridColor);
    }
    for(int i = 0; i< gridRows + 1; i++) 
    {
        vbGrid[gridColumns*2+2+i*2] = sf::Vertex(topLeftPoint + sf::Vector2f(0,gridSquareSize*i), GridColor);
        vbGrid[gridColumns*2+2+i*2+1] = sf::Vertex(topLeftPoint + sf::Vector2f(gridRowSize,gridSquareSize*i), GridColor);
    }
}

sf::Vector2f translateScreenToGridCords(sf::Vector2f screenCords) {
    return (screenCords - gridTopLeftCord)/gridSquareSize;
}

///Rendered Game Objects

enum BlockType 
{
    square, circle
};

//
class BlockSprite 
{
public:
    BlockType blockType;
    sf::Color blockColor;
    float scaleWidth; //Ranges from 0.0f -> 1.0f or more
    float scaleHeight; //Ranges from 0.0f -> 1.0f or more

    BlockSprite(BlockType blockType, sf::Color blockColor, float scaleWidth, float scaleHeight) : blockType(blockType), blockColor(blockColor), scaleWidth(scaleWidth), scaleHeight(scaleHeight) {}

    void draw(sf::RenderWindow& window, sf::Vector2f LeftTop, sf::Vector2f RightBot) const 
    {
        if(blockType == BlockType::square) 
        {
            sf::RectangleShape square;
            auto delta = (RightBot-LeftTop);
            auto size = sf::Vector2f(delta.x * scaleWidth, delta.y * scaleHeight);
            square.setSize(size);
            square.setPosition(LeftTop + (delta - size) / 2.0f);
            square.setFillColor(blockColor);
            window.draw(square);
        }

        if(blockType == BlockType::circle) 
        {
            sf::CircleShape circle;
            auto delta = (RightBot-LeftTop);
            auto size = sf::Vector2f(delta.x * scaleWidth, delta.y * scaleHeight);
            circle.setPosition(LeftTop + (delta - size) / 2.0f);
            circle.setRadius(scaleWidth*delta.x/2.0f);
            circle.setFillColor(blockColor);
            window.draw(circle);
        }
    }
};

//Fixed Sprites to use for rendering
BlockSprite stoneSprite = BlockSprite(BlockType::square, sf::Color(100,100,100), 0.9f, 0.9f);
BlockSprite playerSprite = BlockSprite(BlockType::circle, sf::Color(255,0,0), 0.9f, 0.9f);
BlockSprite goalSprite = BlockSprite(BlockType::circle, sf::Color(255,255,0), 0.8f, 0.8f);

BlockSprite pathSprite = BlockSprite(BlockType::square, sf::Color(180,0,0,80), 0.6f, 0.6f);
BlockSprite goalPathSprite = BlockSprite(BlockType::square, sf::Color(190,190,0,150), 0.4f, 0.4f);
BlockSprite backFronteirSprite = BlockSprite(BlockType::square, sf::Color(186,85,211,200), 0.8f, 0.8f);

//Game & Algorithm
std::unique_ptr<TransistionModel> gameModel;
MapState currentMapState(10,10);

SearchAlgorithm searchAlgorithm;
std::thread moveThread;
std::thread searchThread;

//Draws an object on the grid using grid-cords
void drawBlockObjectOnGrid(sf::RenderWindow& window, const BlockSprite* obj, sf::Vector2f cords) 
{
    auto blockTopLeft = gridTopLeftCord + (float)gridSquareSize * cords;
    auto blockBotRight = gridTopLeftCord + (float)gridSquareSize * (cords + sf::Vector2f(1,1));
    obj->draw(window, blockTopLeft, blockBotRight);
}

void drawMapState(sf::RenderWindow& window, const MapState& state) {
    gridRows = state.width;
    gridColumns = state.height;
    for(int x = 0; x<gridRows; x++) {
        for(int y = 0; y<gridColumns; y++) {
            Object obj = state.getObject(sf::Vector2i(x,y));
            if(obj == Object::stone) drawBlockObjectOnGrid(window, &stoneSprite, sf::Vector2f(x,y));
        }
    }
    drawBlockObjectOnGrid(window, &playerSprite, sf::Vector2f(state.playerPos.x,state.playerPos.y));
    if(searchAlgorithm.frontier.size() > 0 && searchAlgorithm.getFrontierNode().init) {
        drawBlockObjectOnGrid(window, &backFronteirSprite, sf::Vector2f(searchAlgorithm.getFrontierNode().mapState.playerPos));
    }
}

void setCurrentMapState(const MapState& mapstate) {
    currentMapState = mapstate;
    generateGrid(currentMapState);
}

//Make player do actions slowly, used in thread
void doActions(std::vector<Action> actions) {
    for(Action action : actions) {
        setCurrentMapState(gameModel->result(currentMapState, action));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
}

//Draw extra things
std::vector<std::pair<BlockSprite*, sf::Vector2f>> extraDraw;
void addExtraDraw(BlockSprite* sprite, sf::Vector2f cord) {
    extraDraw.push_back(std::pair<BlockSprite*, sf::Vector2f>(sprite, cord));
}

void drawExtra(sf::RenderWindow& window) {
    for(std::pair<BlockSprite*, sf::Vector2f> pair : extraDraw) {
        auto sprite = pair.first;
        auto cord = pair.second;
        drawBlockObjectOnGrid(window, sprite, cord);
    }
    MapState agent = searchAlgorithm.initStateObj;
    for(Action action : searchAlgorithm.goalNode.actionPath) {
        agent = gameModel->result(agent, action);
        drawBlockObjectOnGrid(window, &goalPathSprite, sf::Vector2f(agent.playerPos));
    }
    drawBlockObjectOnGrid(window, &goalPathSprite, sf::Vector2f(agent.playerPos));
}

//Slow algorithm search used in thread
void doSlowSearch() {
    SearchState searchState = SearchState::Searching;
    while(searchState == SearchState::Searching) {
        searchState = searchAlgorithm.searchDeepTick();
        switch (searchState)
        {
        case SearchState::Searching:
            std::cout << "Searching!!";
            break;
        case SearchState::Found:
            std::cout << "Found!!";
            break;
        case SearchState::Failure:
            std::cout << "Failed!!";
            break;
        default:
            break;
        }
        if(searchState != SearchState::Failure) {
            auto playerPos = searchAlgorithm.getFrontierNode().mapState.playerPos;
            addExtraDraw(&pathSprite, sf::Vector2f(playerPos.x, playerPos.y));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}


int main() 
{
    sf::RenderWindow window(sf::VideoMode(WindowWidth, WindowHeight), "SFML window");
    
    //Init game
    gameModel = std::unique_ptr<TransistionModel>(new MazeModel());
    setCurrentMapState(gameModel->init());

    

    // Main loop
    while (window.isOpen()) 
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) 
        {
            // Close window: exit
            if (event.type == sf::Event::Closed)
                window.close();
            if(event.type == sf::Event::Resized) 
            {
                windowHeight = event.size.height;
                windowWidth = event.size.width;
                windowCenterCord = sf::Vector2f(windowWidth/2, windowHeight/2);
                if(windowWidth < windowHeight) gridSize = windowWidth - 20;
                if(windowWidth > windowHeight) gridSize = windowHeight - 20;
                window.setView(sf::View(windowCenterCord, sf::Vector2f(windowWidth, windowHeight)));
                generateGrid(currentMapState);
            }
            if(event.type == sf::Event::MouseButtonPressed)
            {
                sf::Vector2f gridCordFloat = translateScreenToGridCords(sf::Vector2f(event.mouseButton.x, event.mouseButton.y));
                sf::Vector2i gridCord = sf::Vector2i(std::floor(gridCordFloat.x), std::floor(gridCordFloat.y));

                if(event.mouseButton.button == sf::Mouse::Button::Left) {
                    auto state = currentMapState;
                    state.setObject(gridCord, Object::stone);
                    setCurrentMapState(state);
                }
                if(event.mouseButton.button == sf::Mouse::Button::Right) {
                    auto state = currentMapState;
                    state.setObject(gridCord, Object::air);
                    setCurrentMapState(state);
                }
            }
            if(event.type == sf::Event::KeyPressed) 
            {
                switch(event.key.code) 
                {
                    case sf::Keyboard::D: 
                    {
                        setCurrentMapState(gameModel->result(currentMapState, Action::MoveRight));
                        break;
                    };
                    case sf::Keyboard::A: 
                    {
                        setCurrentMapState(gameModel->result(currentMapState, Action::MoveLeft));
                        break;
                    };
                    case sf::Keyboard::W: 
                    {
                        setCurrentMapState(gameModel->result(currentMapState, Action::MoveUp));
                        break;
                    };
                    case sf::Keyboard::S: 
                    {
                        setCurrentMapState(gameModel->result(currentMapState, Action::MoveDown));
                        break;
                    };
                    case sf::Keyboard::Z: 
                    {
                        searchAlgorithm = SearchAlgorithm(currentMapState, gameModel.get());
                        auto resultState = searchAlgorithm.searchResult();
                        switch (resultState)
                        {
                        case SearchState::Found:
                            std::cout << "Found!!";
                            break;
                        case SearchState::Failure:
                            std::cout << "Failed!!";
                            break;
                        default:
                            break;
                        }
                        break;
                    };
                    case sf::Keyboard::X: 
                    {
                        if (moveThread.joinable()) {
                            moveThread.join();
                        }
                        moveThread = std::thread(doActions, searchAlgorithm.goalNode.actionPath);
                        break;
                    };
                    case sf::Keyboard::C:
                    {

                        extraDraw.clear();
                        if (searchThread.joinable()) {
                            searchThread.join();
                        }
                        searchAlgorithm = SearchAlgorithm(currentMapState, gameModel.get());
                        searchThread = std::thread(doSlowSearch);
                        break;
                    }
                }
                
            }
        }

    
        window.clear(BackgroundColor);
        window.draw(vbGrid);

        drawMapState(window,currentMapState);
        drawExtra(window);

        window.display();
        
    }
    moveThread.join();
    return 0;
}
