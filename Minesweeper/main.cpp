#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <random>
#include <cstdlib>
#include <chrono>
#include <queue>
#include <string>
#include <set>

enum class GameState {
    InProgress,
    Win,
    Lose,
    Paused
};
enum class TileState {
    Hidden,
    Flagged,
    Revealed
};

void display(const std::vector<std::vector<int>> &v) {
    for (auto &i: v) {
        for (int j: i) {
            std::cout << std::setw(4) << j << " ";
        }
        std::cout << std::endl;
    }
}

void getWindowDimen(int &width, int &height, int &mineCount, int &tileCount) {
    std::ifstream configFile("files/board_config.cfg");

    int numColumns = 0;
    int numRows = 0;
    configFile >> numColumns >> numRows >> mineCount >> tileCount;
    width = numColumns * 32;
    height = (numRows * 32) + 100;
}


void setText(sf::Text &text, float x, float y) {
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    text.setPosition(sf::Vector2f(x, y));
}

// (r-1)(c-1)       (r-1)c      (r-1)(c+1)
// r(c-1)           rc          r(c+1)
// (r+1)(c-1)       (r+1)c      (r+1)(c+1)
void
revealEmptyTiles(std::vector<std::vector<int>> &board, std::vector<std::vector<TileState>> &tileStates, const int &row,
                 const int &col, int &tilesRevealed) {
    std::queue<std::pair<int, int>> q;
    std::set<std::pair<int, int>> visited;
    q.emplace(row, col);    //Pushing the current tile
    visited.insert({row, col});
    while (!q.empty()) {
        auto pos = q.front();
        q.pop();
        int r = pos.first, c = pos.second;
        if (tileStates[r][c] == TileState::Revealed || board[r][c] == -1) {
            continue;
        }
        tileStates[r][c] = TileState::Revealed;
        tilesRevealed++;
        if (board[r][c] == 0) {
            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    int nr = r + dr, nc = c + dc;
                    if (nr < 0 || nr >= board.size() || nc < 0 || nc >= board[0].size()) {
                        continue;
                    }
                    if (visited.count({nr, nc}) == 0) {
                        q.emplace(nr, nc);      //Pushing the neighbors of the current tile
                        visited.insert({nr, nc});
                    }
                }
            }
        }
    }
}


void initGame(std::vector<std::vector<int>> &board, std::vector<std::vector<TileState>> &tileStates, const int &numRows,
              const int &numCols, const int &mineCount) {
    std::vector<std::vector<int>> gameBoard(numRows, std::vector<int>(numCols, 0));
    std::vector<std::vector<TileState>> tileState(numRows, std::vector<TileState>(numCols, TileState::Hidden));
    // Initialize the game board with random mine placement
    // Use a random_device to generate a seed for the random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    for (int i = 0; i < mineCount; i++) {
        int row, col;
        do {
            row = std::uniform_int_distribution<int>(0, numRows - 1)(gen);
            col = std::uniform_int_distribution<int>(0, numCols - 1)(gen);
        } while (gameBoard[row][col] == -1);
        gameBoard[row][col] = -1;
        // Increment the surrounding tiles
        for (int j = row - 1; j <= row + 1; j++) {
            for (int k = col - 1; k <= col + 1; k++) {
                if (j >= 0 && j < numRows && k >= 0 && k < numCols && gameBoard[j][k] != -1) {
                    gameBoard[j][k]++;
                }
            }
        }
    }
    board = gameBoard;
    tileStates = tileState;
}


void
displayLeaderBoard(const int &width, const int &height, const sf::Font &font, const std::string &playerName) {
    // Create the SFML window
    sf::RenderWindow leaderBoardWin(sf::VideoMode(width / 2, height / 2), "Leaderboard",
                                    sf::Style::Titlebar | sf::Style::Close);
    leaderBoardWin.setFramerateLimit(60);

    // Set up the leaderboard text
    sf::Text leaderboardText;
    leaderboardText.setFont(font);
    leaderboardText.setCharacterSize(18);
    leaderboardText.setStyle(sf::Text::Bold);
    leaderboardText.setFillColor(sf::Color::White);

    // Open the leaderboard file and read its contents
    std::ifstream leaderboardFile("files/leaderboard.txt");
    std::string row;
    std::string leaderboardContent;
    char idx = '1';
    int count = 0;
    while (count < 5 && std::getline(leaderboardFile, row)) {
        //If new score is inserted
        std::string end = "\n\n";
        if (row.substr(6, std::string::npos) == playerName)
            end = "*\n\n";
        row = ".\t" + row.substr(0, 5) + "\t" + row.substr(6, std::string::npos) + end;
        std::string t = idx + row;
        row = t;
        idx++;
        leaderboardContent += row;
        count++;
    }
    // Set the leaderboard text content and position
    leaderboardText.setString(leaderboardContent);
    setText(leaderboardText, leaderBoardWin.getSize().x / 2.0f, leaderBoardWin.getSize().y / 2.0f + 20);

    // Set up the leaderboard title text
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setCharacterSize(20);
    titleText.setStyle(sf::Text::Bold | sf::Text::Underlined);
    titleText.setString("LEADERBOARD");
    titleText.setFillColor(sf::Color::White);

    // Set the leaderboard title position
    setText(titleText, leaderBoardWin.getSize().x / 2.0f, leaderBoardWin.getSize().y / 2.0f - 120);
    // Run the SFML loop
    while (leaderBoardWin.isOpen()) {
        // Handle SFML events
        sf::Event event;
        while (leaderBoardWin.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                leaderBoardWin.close();
            }
        }

        // Clear the window
        leaderBoardWin.clear(sf::Color::Blue);
        // Draw the leaderboard text and title
        leaderBoardWin.draw(leaderboardText);
        leaderBoardWin.draw(titleText);
        // Display the window
        leaderBoardWin.display();
    }

}

void insert_score(const int &t, const std::string &name, const bool& called) {
    if (called)
        return;
    int mins = t / 60;
    int secs = t % 60;
    std::string time="00:00";
    if (mins < 10) {
        time[0] = '0';
        time[1] = mins + '0';
    } else {
        time[0] = mins / 10 + '0';
        time[1] = mins % 10 + '0';
    }
    if (secs < 10) {
        time[3] = '0';
        time[4] = secs + '0';
    } else {
        time[3] = secs / 10 + '0';
        time[4] = secs % 10 + '0';
    }
    // Open the leaderboard file and read its contents into a vector of strings
    std::ifstream leaderboardFile("files/leaderboard.txt");
    std::vector<std::string> leaderboardContent;
    std::string line;
    while (std::getline(leaderboardFile, line)) {
        leaderboardContent.push_back(line);
    }
    leaderboardFile.close();

    // Insert the new score into the appropriate position in the vector
    bool inserted = false;
    for (auto it = leaderboardContent.begin(); it != leaderboardContent.end(); ++it) {
        std::string currentTime = it->substr(0, 5);
        int minsTime = std::stoi(currentTime.substr(0, 2));
        int secsTime = std::stoi(currentTime.substr(3, 2));
        if (mins < minsTime || (mins == minsTime && secs < secsTime)) {
            leaderboardContent.insert(it, time + "," + name);
            inserted = true;
            break;
        }
    }
    if (!inserted) {
        leaderboardContent.push_back(time + "," + name);
    }

    // Open the leaderboard file and write the updated contents to it
    std::ofstream leaderboardFileOut("files/leaderboard.txt");
    for (const auto &line: leaderboardContent) {
        leaderboardFileOut << line << std::endl;
    }
    leaderboardFileOut.close();
}


int main() {
    int width, height, mineCount, tileCount;
    getWindowDimen(width, height, mineCount, tileCount);
    const int MINE_COUNT = mineCount;
    sf::RenderWindow window(sf::VideoMode(width, height), "Welcome Window", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("files/font.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
        return 1;
    }

    // Welcome Text
    sf::Text welcomeText("WELCOME TO MINESWEEPER!", font, 24);
    welcomeText.setStyle(sf::Text::Bold | sf::Text::Underlined);
    welcomeText.setFillColor(sf::Color::White);
    setText(welcomeText, (float) window.getSize().x / 2.0f, (float) window.getSize().y / 2.0f - 150);

    // Input Prompt Text
    sf::Text inputPromptText("Enter your name:", font, 20);
    inputPromptText.setStyle(sf::Text::Bold);
    inputPromptText.setFillColor(sf::Color::White);
    setText(inputPromptText, (float) window.getSize().x / 2.0f, (float) window.getSize().y / 2.0f - 75);

    // Input Text
    sf::Text inputText("", font, 18);
    inputText.setStyle(sf::Text::Bold);
    inputText.setFillColor(sf::Color::Yellow);
    setText(inputText, (float) window.getSize().x / 2.0f, (float) window.getSize().y / 2.0f - 45);

    sf::RectangleShape cursorBg(sf::Vector2f(2.0f, inputText.getGlobalBounds().height));
    cursorBg.setFillColor(sf::Color::Transparent);

    std::string name = "|";

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return 0;
            }
            inputText.setString(name);
            if (event.type == sf::Event::TextEntered) {
                if ((event.text.unicode >= 'A' && event.text.unicode <= 'Z') ||
                    (event.text.unicode >= 'a' && event.text.unicode <= 'z') && name.size() < 11) {
                    name.pop_back();
                    name += static_cast<char>(event.text.unicode);
                    name.push_back('|');     //Extra character for cursor
                    inputText.setString(name);
                    setText(inputText, (float) window.getSize().x / 2.0f, (float) window.getSize().y / 2.0f - 35);
                } else if (event.text.unicode == '\b') {
                    if (!name.empty()) {
                        name.pop_back();    //To pop the cursor
                        name.pop_back();    //To pop the character
                        inputText.setString(name);
                        name.push_back('|');
                        setText(inputText, (float) window.getSize().x / 2.0f, (float) window.getSize().y / 2.0f - 35);
                    }
                }
            }

            if (event.type == sf::Event::KeyPressed) {
                if (name.size() > 1 && event.key.code == sf::Keyboard::Enter) {
                    name.pop_back();
                    window.close();
                }
            }
        }

//        cursorBg.setPosition(inputText.getPosition().x - 5.0f, window.getSize().y / 2.0f - 45);
        cursorBg.setSize(
                sf::Vector2f(inputText.getGlobalBounds().width + 10.0f, inputText.getGlobalBounds().height + 10.0f));

        window.clear(sf::Color::Blue); // set the background color to blue
        window.draw(welcomeText);
        window.draw(inputPromptText);
        window.draw(inputText);

        window.display();
    }

    // capitalize first letter, lowercase the rest
    if (!name.empty()) {
        name[0] = (char) std::toupper(name[0]);
        for (int i = 1; i < name.size(); i++) {
            name[i] = (char) std::tolower(name[i]);
        }
    }

    /**
     * Game window setup
     */
    //Grid size:
    const int numRows = (height - 100) / 32;
    const int numCols = width / 32;
    // Create the game window
    sf::RenderWindow gameWindow(sf::VideoMode(width, height), "Minesweeper", sf::Style::Titlebar | sf::Style::Close);
    gameWindow.setFramerateLimit(60);
    std::vector<std::vector<int>> gameBoard(numRows, std::vector<int>(numCols, 0));
    std::vector<std::vector<TileState>> tileState(numRows, std::vector<TileState>(numCols, TileState::Hidden));
    // Initialize the game
    GameState gameState = GameState::InProgress;
    bool addedNewScore = false;
    initGame(gameBoard, tileState, numRows, numCols, MINE_COUNT);
    // For debugging
    display(gameBoard);



    /**
     * Loading the textures
     */
    std::vector<sf::Texture> numberTextures;
    for (int i = 1; i <= 8; i++) {
        sf::Texture texture;
        if (!texture.loadFromFile("files/images/number_" + std::to_string(i) + ".png")) {
            std::cerr << "Failed to load number texture " << i << "!" << std::endl;
            return 1;
        }
        numberTextures.push_back(texture);
    }

    sf::Texture mineTexture;
    if (!mineTexture.loadFromFile("files/images/mine.png")) {
        std::cerr << "Failed to load mine texture!" << std::endl;
        return 1;
    }
    sf::Texture hiddenTexture;
    if (!hiddenTexture.loadFromFile("files/images/tile_hidden.png")) {
        std::cerr << "Failed to load hidden tile texture!" << std::endl;
        return 1;
    }
    sf::Texture revealedTexture;
    if (!revealedTexture.loadFromFile("files/images/tile_revealed.png")) {
        std::cerr << "Failed to load revealed tile texture!" << std::endl;
        return 1;
    }

    sf::Texture happyFaceTexture;
    if (!happyFaceTexture.loadFromFile("files/images/face_happy.png")) {
        std::cerr << "Failed to load happy face texture!" << std::endl;
        return 1;
    }
    sf::Texture winFaceTexture;
    if (!winFaceTexture.loadFromFile("files/images/face_win.png")) {
        std::cerr << "Failed to load win face texture!" << std::endl;
        return 1;
    }
    sf::Texture loseFaceTexture;
    if (!loseFaceTexture.loadFromFile("files/images/face_lose.png")) {
        std::cerr << "Failed to load lose face texture!" << std::endl;
        return 1;
    }
    sf::Texture digitsTexture;
    if (!digitsTexture.loadFromFile("files/images/digits.png")) {
        std::cerr << "Failed to load digits texture!" << std::endl;
        return 1;
    }
    sf::Texture debugTexture;
    if (!debugTexture.loadFromFile("files/images/debug.png")) {
        std::cerr << "Failed to load debug texture!" << std::endl;
        return 1;
    }
    sf::Texture pauseTexture;
    if (!pauseTexture.loadFromFile("files/images/pause.png")) {
        std::cerr << "Failed to load pause texture!" << std::endl;
        return 1;
    }
    sf::Texture playTexture;
    if (!playTexture.loadFromFile("files/images/play.png")) {
        std::cerr << "Failed to load play texture!" << std::endl;
        return 1;
    }
    sf::Texture flagTexture;
    if (!flagTexture.loadFromFile("files/images/flag.png")) {
        std::cerr << "Failed to load flag texture!" << std::endl;
        return 1;
    }
    sf::Texture leaderboardTexture;
    if (!leaderboardTexture.loadFromFile("files/images/leaderboard.png")) {
        std::cerr << "Failed to load leaderboard texture!" << std::endl;
        return 1;
    }
    sf::Sprite mineSprite(mineTexture);
    sf::Sprite hiddenSprite(hiddenTexture);
    sf::Sprite revealedSprite(revealedTexture);
    std::vector<sf::Sprite> numberSprites;
    sf::Sprite happyFaceSprite(happyFaceTexture);
    sf::Sprite winFaceSprite(winFaceTexture);
    sf::Sprite loseFaceSprite(loseFaceTexture);
    sf::Sprite digitsSprite(digitsTexture);
    sf::Sprite debugSprite(debugTexture);
    sf::Sprite pauseSprite(pauseTexture);
    sf::Sprite playSprite(playTexture);
    sf::Sprite flagSprite(flagTexture);
    sf::Sprite leaderBoardSprite(leaderboardTexture);
    sf::Sprite timerSprite(digitsTexture);
    for (int i = 0; i < 8; i++) {
        sf::Sprite sprite(numberTextures[i]);
        numberSprites.push_back(sprite);
    }
    // Start the timer
    auto start_time = std::chrono::high_resolution_clock::now();
    // Elapsed time
    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::high_resolution_clock::now() - start_time).count();

    int tilesRevealed = 0;
    // For debugging
    bool isDebugging = false;
    auto pauseTime = std::chrono::high_resolution_clock::now();
    //Main looper
    while (gameWindow.isOpen()) {
        sf::Event event{};
        while (gameWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                gameWindow.close();
            }
        }
        /**
         * Click listeners
         */
        if (gameState == GameState::InProgress) {
            // Handle left click
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                //If the click was in between tiles
                if (event.mouseButton.y <= height - 100) {
                    int row = event.mouseButton.y / 32; // Calculate row based on mouse y-coordinate
                    int col = event.mouseButton.x / 32; // Calculate column based on mouse x-coordinate
                    if (tileState[row][col] != TileState::Flagged) { // Only reveal tile if it is not flagged
                        if (gameBoard[row][col] == -1) { // Bomb tile
                            // Reveal all bomb tiles
                            for (int i = 0; i < numRows; i++) {
                                for (int j = 0; j < numCols; j++) {
                                    if (gameBoard[i][j] == -1) {
                                        tileState[i][j] = TileState::Revealed;
                                        tilesRevealed++;
                                    }
                                }
                            }
                            gameState = GameState::Lose;
                        } else if (gameBoard[row][col] == 0) { // Empty tile
                            //Reveal all adjacent empty tiles
                            revealEmptyTiles(gameBoard, tileState, row, col, tilesRevealed);
                        } else { // Number tile
                            tileState[row][col] = TileState::Revealed;
                            tilesRevealed++;
                        }
                    }
                }
            }
            // Handle right click
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
                if (event.mouseButton.y <= height - 100) {
                    int row = event.mouseButton.y / 32; // Calculate row based on mouse y-coordinate
                    int col = event.mouseButton.x / 32; // Calculate column based on mouse x-coordinate
                    if (tileState[row][col] == TileState::Hidden) {
                        tileState[row][col] = TileState::Flagged;
                        mineCount--; // Decrease mine count
                    } else if (tileState[row][col] == TileState::Flagged) {
                        tileState[row][col] = TileState::Hidden;
                        mineCount++; // Increase mine count
                    }
                }
            }
        }
        // Check if the player has won
        if (tilesRevealed == (numRows * numCols) - mineCount) {
            gameState = GameState::Win;
            elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::high_resolution_clock::now() - start_time).count();
            int time_elapsed = (int) elapsed_time;
            insert_score(time_elapsed, name, addedNewScore);
            addedNewScore = true;
        }
        // Set the background color of the game window to white
        gameWindow.clear(sf::Color::White);
        // Draw the tiles
        for (int i = 0; i < numRows; i++) {
            for (int j = 0; j < numCols; j++) {
                sf::Sprite sprite = hiddenSprite;
                if (gameState == GameState::Paused) {
                    sprite = revealedSprite;
                    sprite.setPosition((float) j * 32.0f, (float) i * 32.0f);
                    gameWindow.draw(sprite);
                    continue;
                }
                // Determine which sprite to use for the tile

                if (tileState[i][j] == TileState::Revealed) {
                    sprite = revealedSprite;
                    // Set the position of the sprite
                    sprite.setPosition((float) j * 32.0f, (float) i * 32.0f);
                    // Draw the sprite
                    gameWindow.draw(sprite);
                    if (gameBoard[i][j] > 0) {
                        // Determine which number sprite to use for the tile
                        sprite = numberSprites[gameBoard[i][j] - 1];
                    }
                    if (gameBoard[i][j] == -1) {
                        sprite = mineSprite;
                    }
                } else {
                    sprite = hiddenSprite;
                    //If the game is in debug mode then draw the mines, too.
                    if (isDebugging) {
                        // Set the position of the sprite
                        sprite.setPosition((float) j * 32.0f, (float) i * 32.0f);
                        // Draw the sprite
                        gameWindow.draw(sprite);
                        if (gameBoard[i][j] == -1) {
                            sprite = mineSprite;
                        }
                    }
                }
                // Set the position of the sprite
                sprite.setPosition((float) j * 32.0f, (float) i * 32.0f);
                // Draw the sprite
                gameWindow.draw(sprite);
                //if it is Flagged then first draw the hiddenSpite then on top of it draw the flagSprite
                if (tileState[i][j] == TileState::Flagged) {
                    flagSprite.setPosition((float) j * 32.0f, (float) i * 32.0f);
                    gameWindow.draw(flagSprite);
                }
            }
        }
        // Draw the face button
        sf::Sprite faceSprite = happyFaceSprite;
        if (gameState == GameState::Win) {
            faceSprite = winFaceSprite;
        } else if (gameState == GameState::Lose) {
            faceSprite = loseFaceSprite;
        }
        faceSprite.setPosition(((float) numCols / 2.0f * 32.0f) - 32.0f, 32.0f * ((float) numRows + 0.5f));
        gameWindow.draw(faceSprite);

        // Draw the debug button
        debugSprite.setPosition((float) numCols * 32.0f - 304.0f, 32.0f * ((float) numRows + 0.5f));
        gameWindow.draw(debugSprite);

        // Draw the pause/play button
        sf::Sprite pausePlaySprite = playSprite;
        if (gameState == GameState::Paused) {
            pausePlaySprite = pauseSprite;
        }
        pausePlaySprite.setPosition((float) numCols * 32.0f - 240.0f, 32.0f * ((float) numRows + 0.5f));
        gameWindow.draw(pausePlaySprite);
        // Draw the leaderboard button
        leaderBoardSprite.setPosition((float) numCols * 32.0f - 176.0f, 32.0f * ((float) numRows + 0.5f));
        gameWindow.draw(leaderBoardSprite);

        //Click listeners for bottom buttons
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            // Check if the click was on the face button
            if (faceSprite.getGlobalBounds().contains((float) event.mouseButton.x, (float) event.mouseButton.y)) {
                //Restart the game
                initGame(gameBoard, tileState, numRows, numCols, MINE_COUNT);
                tilesRevealed = 0;
                addedNewScore = false;
                isDebugging = false;
                gameState = GameState::InProgress;
                mineCount = MINE_COUNT;
                start_time = std::chrono::high_resolution_clock::now();
            }
            // Check if the click was on the debug button
            if (debugSprite.getGlobalBounds().contains((float) event.mouseButton.x, (float) event.mouseButton.y)) {
                isDebugging = !isDebugging;
            }
            // Check if the click was on the pause/play button
            if (pausePlaySprite.getGlobalBounds().contains((float) event.mouseButton.x,
                                                           (float) event.mouseButton.y)) {
                if (gameState == GameState::Paused) {
                    gameState = GameState::InProgress;
                    start_time += std::chrono::high_resolution_clock::now() - pauseTime;
                } else if (gameState == GameState::InProgress) {
                    gameState = GameState::Paused;
                    //Note down the time when the game was paused
                    pauseTime = std::chrono::high_resolution_clock::now();
                }
            }
            // Check if the click was on the leaderboard button
            if (leaderBoardSprite.getGlobalBounds().contains((float) event.mouseButton.x,
                                                             (float) event.mouseButton.y)) {
                //reveals/hides the leaderboard window
                gameState = GameState::Paused;
                pauseTime = std::chrono::high_resolution_clock::now();
                //**********Leaderboard Window**********
                displayLeaderBoard(width, height, font, name);
                start_time += std::chrono::high_resolution_clock::now() - pauseTime;
                gameState = GameState::InProgress;
            }
        }

        // Draw the mine counter
        sf::Vector2f counterPos(33.0f, 32 * ((float) numRows + 0.5f) + 16);
        std::string counter = "000";
        int tCount = abs(mineCount);
        int idx = 2;
        while (idx >= 0) {
            int digit = tCount % 10;
            counter[idx--] = (char) ('0' + digit);
            tCount /= 10;
        }
        tCount = mineCount;
        if (tCount < 0) {
            // Draw the negative sign sprite
            sf::Sprite negativeSprite(digitsTexture);
            negativeSprite.setTextureRect(sf::IntRect(10 * 21, 0, 21, 32));
            negativeSprite.setPosition(counterPos.x, counterPos.y);
            gameWindow.draw(negativeSprite);
            // Update the position for the next digit sprite
            counterPos.x += 21;
            // Make remainingMines positive for drawing the digits
            tCount = -tCount;
        }
        // Draw each digit of the mine counter

        idx = 0;
        while (idx < 3) {
            int digit = counter[idx++] - '0';
            sf::Sprite digitSprite(digitsTexture);
            digitSprite.setTextureRect(sf::IntRect(digit * 21, 0, 21, 32));
            digitSprite.setPosition(counterPos.x, counterPos.y);
            gameWindow.draw(digitSprite);
            // Update the position for the next digit sprite
            counterPos.x += 21;
            tCount /= 10;
        }

        // Calculate elapsed time
        if (gameState == GameState::InProgress) {
            elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::high_resolution_clock::now() - start_time).count();
        }
        int timeElapsed = (int) elapsed_time;
        // Draw the timer
        int minutes = timeElapsed / 60;
        int seconds = timeElapsed % 60;
        sf::Vector2f minutesPos(((float) numCols * 32.0f) - 97, 32 * ((float) numRows + 0.5f) + 16);
        sf::Vector2f secondsPos(((float) numCols * 32.0f) - 54, 32 * ((float) numRows + 0.5f) + 16);
        // Draw the minutes digits
        int digit = minutes / 10;
        sf::Sprite digitSprite(digitsTexture);
        digitSprite.setTextureRect(sf::IntRect(digit * 21, 0, 21, 32));
        digitSprite.setPosition(minutesPos.x, minutesPos.y);
        gameWindow.draw(digitSprite);
        digit = minutes % 10;
        digitSprite.setTextureRect(sf::IntRect(digit * 21, 0, 21, 32));
        digitSprite.setPosition(minutesPos.x + 21, minutesPos.y);
        gameWindow.draw(digitSprite);
        // Draw the seconds digits
        digit = seconds / 10;
        digitSprite.setTextureRect(sf::IntRect(digit * 21, 0, 21, 32));
        digitSprite.setPosition(secondsPos.x, secondsPos.y);
        gameWindow.draw(digitSprite);
        digit = seconds % 10;
        digitSprite.setTextureRect(sf::IntRect(digit * 21, 0, 21, 32));
        digitSprite.setPosition(secondsPos.x + 21, secondsPos.y);
        gameWindow.draw(digitSprite);

        // Display everything that has been drawn
        gameWindow.display();
    }
    return 0;
}
