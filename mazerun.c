#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#define TILE_SIZE 32
#define MAZE_WIDTH 20
#define MAZE_HEIGHT 20
#define screenWidth (MAZE_WIDTH * TILE_SIZE)
#define screenHeight (MAZE_HEIGHT * TILE_SIZE)
#define MAX_LEVELS 5

typedef struct {
    int x, y;
} Point;

typedef struct Node {
    Point position;
    struct Node* parent;
} Node;

typedef enum {
    MENU, GAMEPLAY, LEVEL_COMPLETE, GAME_OVER
} GameState;

int currentLevel = 0;
int playerScore = 0;
int highScore = 0;
float levelTime = 0.0f;
float enemyTimer = 0.0f;
bool levelTimeFrozen = false;
bool soundEnabled = true;

Vector2 playerPosition;
Vector2 enemyPosition;
Vector2 levelGoalPositions[MAX_LEVELS] = {
    {18, 18}, {18, 18}, {18, 18}, {18, 18}, {18, 18}
};
Vector2 playerStartPositions[MAX_LEVELS] = {
    {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}
};
Vector2 enemyStartPositions[MAX_LEVELS] = {
    {18, 1}, {18, 1}, {18, 1}, {18, 1}, {18, 1}
};
float enemySpeedPerLevel[MAX_LEVELS] = {
    0.28f, 0.26f, 0.24f, 0.22f, 0.20f
};

GameState gameState = MENU;

Sound soundMove;
Sound soundLevelComplete;
Sound soundMenu;
Sound soundGameOver;
Sound soundEnemy;

Texture2D playerTexture;
Texture2D enemyTexture;
Texture2D goalTexture;


int mazeLevels[MAX_LEVELS][MAZE_HEIGHT][MAZE_WIDTH] = {
    // Level 1 Maze
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,0,1},
        {1,0,1,0,1,0,1,0,0,0,1,0,1,0,0,0,0,1,0,1},
        {1,0,1,0,0,0,1,0,1,0,1,0,1,1,1,1,0,1,0,1},
        {1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,1,0,1,0,1},
        {1,0,1,0,1,0,0,0,1,1,1,1,1,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,1},
        {1,1,1,1,1,0,1,0,1,1,1,1,0,1,0,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1},
        {1,0,1,0,1,1,1,1,1,1,0,0,0,1,1,1,0,1,0,1},
        {1,0,1,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,1,1},
        {1,0,1,1,1,1,1,1,0,0,0,1,1,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,1,0,1},
        {1,1,1,1,1,0,0,1,0,1,1,1,0,0,0,1,0,1,0,1},
        {1,0,0,0,1,1,0,1,0,0,0,1,0,1,0,0,0,1,0,1},
        {1,0,1,0,0,0,0,1,1,1,0,1,0,1,0,1,0,1,0,1},
        {1,0,1,1,1,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1},
        {1,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    //Level 2
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1},
        {1,1,1,1,0,1,0,1,0,1,1,0,1,0,1,1,1,1,0,1},
        {1,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,1,0,1},
        {1,0,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1},
        {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,1},
        {1,0,1,0,1,1,0,1,1,1,1,1,1,1,0,0,0,1,0,1},
        {1,1,1,0,1,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1},
        {1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,1},
        {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,0,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,1},
        {1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,0,1},
        {1,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1},
        {1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,1,1,1,0,1},
        {1,0,1,0,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1},
        {1,0,1,0,1,0,0,0,0,1,0,1,0,0,0,1,1,1,1,1},
        {1,0,1,0,1,1,1,1,0,1,1,1,0,1,0,1,0,0,0,1},
        {1,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Level 3
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,0,1,0,1,1,0,1,0,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1},
        {1,0,1,0,1,0,1,1,0,1,1,0,1,1,1,1,0,1,0,1},
        {1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,1},
        {1,0,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,0,1},
        {1,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,0,1,1,0,1,1,1,1,0,1,1,0,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
        {1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1},
        {1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,0,0,0,0,0,1,0,0,1,0,1,0,1},
        {1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,0,1,0,0,1,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Level 4
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1},
        {1,1,1,1,0,1,0,1,0,1,1,0,1,0,1,1,1,1,0,1},
        {1,0,0,1,0,1,0,0,0,1,0,0,1,0,0,0,0,1,0,1},
        {1,0,1,1,0,1,1,1,0,1,0,1,1,1,0,1,0,0,0,1},
        {1,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,1},
        {1,0,1,0,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1},
        {1,0,1,1,1,0,1,1,1,1,1,1,0,0,0,1,1,1,0,1},
        {1,0,1,0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,1},
        {1,1,1,1,0,1,1,0,1,1,0,1,0,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1,0,0,0,1},
        {1,1,1,1,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,1},
        {1,1,1,0,1,0,1,1,0,0,0,1,1,1,0,1,1,1,1,1},
        {1,0,1,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,1},
        {1,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1},
        {1,1,1,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1},
        {1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    },
    // Level 5
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,0,1},
        {1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1,0,1},
        {1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,0,0,1,0,1},
        {1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1},
        {1,0,1,0,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1},
        {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,0,1,0,1},
        {1,1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,1,1,1,1},
        {1,0,1,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,1},
        {1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,0,0,1},
        {1,0,1,1,0,1,0,1,1,1,0,1,0,1,1,1,0,1,1,1},
        {1,0,1,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1},
        {1,1,1,1,0,1,1,0,1,1,1,1,0,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    }
};

bool IsMouseOver(Rectangle rect) {
    Vector2 mouse = GetMousePosition();
    return CheckCollisionPointRec(mouse, rect);
}

bool IsWalkable(int x, int y) {
    if (x < 0 || y < 0 || x >= MAZE_WIDTH || y >= MAZE_HEIGHT) return false;
    return mazeLevels[currentLevel][y][x] == 0;
}

Vector2 GetNextStepBFS(Vector2 start, Vector2 target) {
    bool visited[MAZE_HEIGHT][MAZE_WIDTH] = { false };
    Node* queue[MAZE_WIDTH * MAZE_HEIGHT];
    int front = 0, rear = 0;

    Node* startNode = malloc(sizeof(Node));
    startNode->position = (Point){ start.x, start.y };
    startNode->parent = NULL;
    queue[rear++] = startNode;
    visited[(int)start.y][(int)start.x] = true;

    Vector2 nextStep = start;

    while (front < rear) {
        Node* current = queue[front++];
        if (current->position.x == (int)target.x && current->position.y == (int)target.y) {
            Node* step = current;
            while (step->parent && step->parent->parent) {
                step = step->parent;
            }
            nextStep.x = step->position.x;
            nextStep.y = step->position.y;
            break;
        }

        Point directions[] = {
            {0, -1}, {0, 1}, {-1, 0}, {1, 0}
        };

        for (int i = 0; i < 4; i++) {
            int nx = current->position.x + directions[i].x;
            int ny = current->position.y + directions[i].y;
            if (IsWalkable(nx, ny) && !visited[ny][nx]) {
                visited[ny][nx] = true;
                Node* next = malloc(sizeof(Node));
                next->position = (Point){nx, ny};
                next->parent = current;
                queue[rear++] = next;
            }
        }
    }

    for (int i = 0; i < rear; i++) free(queue[i]);
    return nextStep;
}

void ResetLevel() {
    playerPosition = playerStartPositions[currentLevel];
    enemyPosition = enemyStartPositions[currentLevel];
    levelTime = 0;
    levelTimeFrozen = false;
    enemyTimer = 0;
}

void UpdateGame() {
    if (gameState == GAMEPLAY) {

        if (!levelTimeFrozen) {
            levelTime += GetFrameTime();
            }

        if (IsKeyPressed(KEY_UP) && IsWalkable(playerPosition.x, playerPosition.y - 1)){
            playerPosition.y--;
            if (soundEnabled) PlaySound(soundMove);
        }
        if (IsKeyPressed(KEY_DOWN) && IsWalkable(playerPosition.x, playerPosition.y + 1)){
            playerPosition.y++;
            if (soundEnabled) PlaySound(soundMove);
        }
        if (IsKeyPressed(KEY_LEFT) && IsWalkable(playerPosition.x - 1, playerPosition.y)){
            playerPosition.x--;
            if (soundEnabled) PlaySound(soundMove);
        }
        if (IsKeyPressed(KEY_RIGHT) && IsWalkable(playerPosition.x + 1, playerPosition.y)){
            playerPosition.x++;
            if (soundEnabled) PlaySound(soundMove);
        }
        // Enemy movement
        enemyTimer += GetFrameTime();
        if (enemyTimer >= enemySpeedPerLevel[currentLevel]) {
            Vector2 next = GetNextStepBFS(enemyPosition, playerPosition);
            enemyPosition = next;
            enemyTimer = 0;
            if (soundEnabled) PlaySound(soundEnemy);
        }

        if ((int)playerPosition.x == (int)enemyPosition.x && (int)playerPosition.y == (int)enemyPosition.y) {
            if (soundEnabled) PlaySound(soundGameOver);
            gameState = GAME_OVER;

        }

        if ((int)playerPosition.x == (int)levelGoalPositions[currentLevel].x &&
            (int)playerPosition.y == (int)levelGoalPositions[currentLevel].y) {
            if (soundEnabled) PlaySound(soundLevelComplete);
            playerScore++;
            if (playerScore > highScore) highScore = playerScore;
            gameState = LEVEL_COMPLETE;
            levelTimeFrozen = true;
            }
        }
    }


void DrawGame() {
    BeginDrawing();
    ClearBackground(BLACK);

    if (gameState == MENU) {
    const char *title = "Maze Runner";
    //const char *startMsg = "Press ENTER to Start";
    const char *soundStatus = soundEnabled ? "Sound: ON" : "Sound: OFF";

    int titleFontSize = 40;
    //int msgFontSize = 20;

    int titleWidth = MeasureText(title, titleFontSize);
    //int msgWidth = MeasureText(startMsg, msgFontSize);

    DrawText(title, (screenWidth - titleWidth)/2, screenHeight/2 - 60, titleFontSize, YELLOW);
    //DrawText(startMsg, (screenWidth - msgWidth)/2, screenHeight/2, msgFontSize, WHITE);

    // ======= Buttons =======
    Rectangle startButton = { (screenWidth - 200)/2, screenHeight/2 + 40, 200, 40 };
    Rectangle soundButton = { (screenWidth - 150)/2, startButton.y + startButton.height + 10, 150, 30 };

    DrawRectangleRec(startButton, SKYBLUE);
    DrawText("Start Game", startButton.x + 40, startButton.y + 10, 20, BLUE);

    DrawRectangleRec(soundButton, LIGHTGRAY);
    DrawText(soundEnabled ? "Sound: ON" : "Sound: OFF", soundButton.x + 10, soundButton.y + 5, 20, GRAY);

    // ======= Input Handling =======
    if (IsKeyPressed(KEY_ENTER) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsMouseOver(startButton))) {
        if (soundEnabled) PlaySound(soundMenu);
        gameState = GAMEPLAY;
        currentLevel = 0;
        playerScore = 0;
        ResetLevel();
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsMouseOver(soundButton)) {
        soundEnabled = !soundEnabled;

        if (!soundEnabled) {
            PauseSound(soundMove);
            PauseSound(soundEnemy);
            PauseSound(soundLevelComplete);
            PauseSound(soundGameOver);
            PauseSound(soundMenu);
        } else {
            ResumeSound(soundMove);
            ResumeSound(soundEnemy);
            ResumeSound(soundLevelComplete);
            ResumeSound(soundGameOver);
            ResumeSound(soundMenu);
        }
    }
}

    else if (gameState == GAMEPLAY) {
        for (int y = 0; y < MAZE_HEIGHT; y++) {
            for (int x = 0; x < MAZE_WIDTH; x++) {
                if (mazeLevels[currentLevel][y][x] == 1)
                    DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, BROWN);
                else
                    DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, BLACK);
            }
        }

        DrawTextureEx(goalTexture,
            (Vector2){levelGoalPositions[currentLevel].x * TILE_SIZE, levelGoalPositions[currentLevel].y * TILE_SIZE},
            0.0f,
            (float)TILE_SIZE / goalTexture.width,WHITE);

        DrawTextureEx(playerTexture,
            (Vector2){playerPosition.x * TILE_SIZE, playerPosition.y * TILE_SIZE},
            0.0f,
            (float)TILE_SIZE / playerTexture.width,WHITE);

        DrawTextureEx(enemyTexture,
            (Vector2){enemyPosition.x * TILE_SIZE, enemyPosition.y * TILE_SIZE},
            0.0f,
            (float)TILE_SIZE / enemyTexture.width,WHITE);



        DrawText(TextFormat("Level: %d", currentLevel + 1), 10, 10, 20, WHITE);
        DrawText(TextFormat("Time: %.1f", levelTime), 10, 40, 20, WHITE);
        DrawText(TextFormat("High Score: %d", highScore), 10, 70, 20, WHITE);
    }
    if (gameState == GAME_OVER) {
    const char *gameOverText = "Game Over!";
    //const char *restartMsg = "Press R to Restart | Press Q to Quit";

    int textFontSize = 40;
    //int msgFontSize = 20;

    int textWidth = MeasureText(gameOverText, textFontSize);
    //int msgWidth = MeasureText(restartMsg, msgFontSize);

    DrawText(gameOverText, (screenWidth - textWidth)/2, screenHeight/2 - 80, textFontSize, RED);
    //DrawText(restartMsg, (screenWidth - msgWidth)/2, screenHeight/2 - 30, msgFontSize, WHITE);

    // Show Score
    char scoreText[50];
    sprintf(scoreText, "Your Score: %d", playerScore);
    int scoreWidth = MeasureText(scoreText, 20);
    DrawText(scoreText, (screenWidth - scoreWidth)/2, screenHeight/2 + 10, 20, WHITE);

    // Restart Button
    Rectangle restartButton = { (screenWidth - 220)/2, screenHeight/2 + 70, 100, 40 };
    DrawRectangleRec(restartButton, SKYBLUE);
    DrawText("Restart", restartButton.x + 10, restartButton.y + 10, 20, BLUE);

    // Quit Button
    Rectangle quitButton = { restartButton.x + 120, screenHeight/2 + 70, 100, 40 };
    DrawRectangleRec(quitButton, SKYBLUE);
    DrawText("Quit", quitButton.x + 30, quitButton.y + 10, 20, BLUE);

    // Input Handling
    if (IsKeyPressed(KEY_R) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsMouseOver(restartButton))) {
        gameState = MENU;
    }

    if (IsKeyPressed(KEY_Q) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsMouseOver(quitButton))) {
        CloseWindow(); // This will close the game window
    }
}

    if (gameState == LEVEL_COMPLETE) {
    const char *completeMsg = "Level Complete!";

    int titleFontSize = 40;

    int completeWidth = MeasureText(completeMsg, titleFontSize);
    DrawText(completeMsg, (screenWidth - completeWidth)/2, screenHeight/2 - 100, titleFontSize, YELLOW);

    // Show Score
    char scoreText[50];
    sprintf(scoreText, "Score: %d", playerScore);
    int scoreWidth = MeasureText(scoreText, 20);
    DrawText(scoreText, (screenWidth - scoreWidth)/2, screenHeight/2 - 40, 20, WHITE);

    // Show Time Taken
    char timeText[50];
    sprintf(timeText, "Time: %.2f seconds", levelTime);
    int timeWidth = MeasureText(timeText, 20);
    DrawText(timeText, (screenWidth - timeWidth)/2, screenHeight/2 - 10, 20, WHITE);

    // Next Level Button
    Rectangle nextButton = { (screenWidth - 220)/2, screenHeight/2 + 40, 100, 40 };
    DrawRectangleRec(nextButton, SKYBLUE);
    DrawText("Next", nextButton.x + 25, nextButton.y + 10, 20, BLUE);

    // Quit Button
    Rectangle quitButton = { nextButton.x + 120, screenHeight/2 + 40, 100, 40 };
    DrawRectangleRec(quitButton, SKYBLUE);
    DrawText("Quit", quitButton.x + 30, quitButton.y + 10, 20, BLUE);

    // Input Handling
    if (IsKeyPressed(KEY_ENTER) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsMouseOver(nextButton))) {
        currentLevel++;
        if (currentLevel >= MAX_LEVELS) {
            gameState = GAME_OVER;
        } else {
            ResetLevel(); // Load next level
            levelTime = 0.0f;  // Reset timer for new level
            gameState = GAMEPLAY;
        }
    }

    if (IsKeyPressed(KEY_Q) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsMouseOver(quitButton))) {
        CloseWindow();
    }
}

    EndDrawing();
}

int main(void) {
    InitWindow(screenWidth, screenHeight, "Maze Runner");
    InitAudioDevice();

    soundMove = LoadSound("assets/move.wav");
    soundLevelComplete = LoadSound("assets/levelcomplete.wav");
    soundGameOver = LoadSound("assets/gameover.wav");
    soundMenu = LoadSound("assets/menu.wav");
    soundEnemy = LoadSound("assets/enemy.wav");

    playerTexture = LoadTexture("assets/player.png");
    enemyTexture = LoadTexture("assets/enemy.png");
    goalTexture = LoadTexture("assets/goal.png");

    SetTargetFPS(60);

   while (!WindowShouldClose()) {

    if (gameState == MENU && IsKeyPressed(KEY_ENTER)) {
        if (soundEnabled) PlaySound(soundMenu);
        gameState = GAMEPLAY;
        currentLevel = 0;
        playerScore = 0;
        ResetLevel();
    }


    if ((gameState == GAME_OVER || gameState == LEVEL_COMPLETE) && IsKeyPressed(KEY_R)) {
        gameState = MENU;
    }

    if (gameState == LEVEL_COMPLETE) {
        if (IsKeyPressed(KEY_ENTER)) {
            if (soundEnabled) PlaySound(soundLevelComplete);
            currentLevel++;
            if (currentLevel < MAX_LEVELS) {
                ResetLevel();
                gameState = GAMEPLAY;
            } else {
                gameState = LEVEL_COMPLETE;
            }
        } else if (IsKeyPressed(KEY_Q)) {

            CloseWindow();
        }
    }

    UpdateGame();
    DrawGame();
}

    UnloadSound(soundMove);
    UnloadSound(soundLevelComplete);
    UnloadSound(soundGameOver);
    UnloadSound(soundMenu);
    UnloadSound(soundEnemy);

    UnloadTexture(playerTexture);
    UnloadTexture(enemyTexture);
    UnloadTexture(goalTexture);

    CloseAudioDevice();

    CloseWindow();
    return 0;
}

