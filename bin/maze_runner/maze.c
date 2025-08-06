
#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define TILE 40
#define W 25
#define H 17

typedef struct { int x, y; } P;

int maze[H][W];
int dx[4] = {1, -1, 0, 0};
int dy[4] = {0, 0, 1, -1};

bool inside(int y, int x) { return x >= 0 && x < W && y >= 0 && y < H; }
bool wall(int y, int x) { return inside(y, x) && maze[y][x] == 1; }

typedef enum { MENU, PLAYING, WIN, GAMEOVER } GameState;
GameState gameState = MENU;

P pl, en;
int enemyTimer = 0;
int level = 1;
int highScore = 1;
int frameCount = 0;

Sound sWin, sOver, sMenu;

int GetEnemySpeed() {
    int speed = 10 - (level > 9 ? 9 : level);
    if (speed < 2) speed = 2;
    return speed;
}

void GenMaze(int difficulty) {
    memset(maze, 0, sizeof(maze));
    for (int i = 0; i < H; i++) for (int j = 0; j < W; j++)
        if (i == 0 || j == 0 || i == H - 1 || j == W - 1) maze[i][j] = 1;

    for (int i = 1; i < H - 1; i++) {
        for (int j = 1; j < W - 1; j++) {
            if (rand() % (10 - difficulty) == 0) maze[i][j] = 1;
        }
    }
    maze[H - 2][W - 2] = 2;
}

void GetNext(P e, P p, P* np) {
    bool vis[H][W] = { 0 };
    P q[H * W]; int qb = 0, qe = 0;
    int pr[H][W], pc[H][W];
    bool found = false;

    q[qe++] = e; vis[e.y][e.x] = true;
    while (qb < qe) {
        P u = q[qb++];
        if (u.x == p.x && u.y == p.y) {
            found = true;
            break;
        }
        for (int i = 0; i < 4; i++) {
            int ny = u.y + dy[i], nx = u.x + dx[i];
            if (inside(ny, nx) && !vis[ny][nx] && maze[ny][nx] != 1) {
                vis[ny][nx] = true;
                pr[ny][nx] = u.y;
                pc[ny][nx] = u.x;
                q[qe++] = (P){ nx, ny };
            }
        }
    }

    if (!found) {
        *np = e;
        return;
    }

    P cur = p;
    while (!(pr[cur.y][cur.x] == e.y && pc[cur.y][cur.x] == e.x)) {
        int ty = pr[cur.y][cur.x], tx = pc[cur.y][cur.x];
        cur = (P){ tx, ty };
    }
    *np = cur;
}

void InitLevel() {
    GenMaze(level);
    pl = (P){ 1, 1 };
    en = (P){ W - 2, 1 };
    enemyTimer = 0;
    frameCount = 0;
}

int main() {
    InitWindow(W * TILE, H * TILE, "Maze Runner Levels");
    InitAudioDevice();

    sWin = LoadSound("assets/win.wav");
    sOver = LoadSound("assets/gameover.wav");
    sMenu = LoadSound("assets/menu.wav");

    SetTargetFPS(30);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (gameState == MENU) {
            const char* title = "MAZE RUNNER";
            const char* prompt = "Press ENTER to Start";
            int titleW = MeasureText(title, 50);
            int promptW = MeasureText(prompt, 20);
            DrawText(title, (W * TILE - titleW) / 2, TILE * 4, 50, ORANGE);
            DrawText(prompt, (W * TILE - promptW) / 2, TILE * 8, 20, RAYWHITE);
            if (IsKeyPressed(KEY_ENTER)) {
                PlaySound(sMenu);
                InitLevel();
                gameState = PLAYING;
            }
        }

        else if (gameState == PLAYING) {
            frameCount++;

            if (IsKeyPressed(KEY_UP) && !wall(pl.y - 1, pl.x)) pl.y--;
            if (IsKeyPressed(KEY_DOWN) && !wall(pl.y + 1, pl.x)) pl.y++;
            if (IsKeyPressed(KEY_LEFT) && !wall(pl.y, pl.x - 1)) pl.x--;
            if (IsKeyPressed(KEY_RIGHT) && !wall(pl.y, pl.x + 1)) pl.x++;

            enemyTimer++;
            if (enemyTimer >= GetEnemySpeed()) {
                P next;
                GetNext(en, pl, &next);
                en = next;
                enemyTimer = 0;
            }

            if (en.x == pl.x && en.y == pl.y) {
                PlaySound(sOver);
                gameState = GAMEOVER;
            }

            if (maze[pl.y][pl.x] == 2) {
                PlaySound(sWin);
                gameState = WIN;
                if (level + 1 > highScore) highScore = level + 1;
            }

            for (int y = 0; y < H; y++) {
                for (int x = 0; x < W; x++) {
                    Color color = (maze[y][x] == 2) ? GREEN :
                                  (maze[y][x] == 1) ? BROWN : BLACK;
                    DrawRectangle(x * TILE, y * TILE, TILE, TILE, color);
                }
            }

            DrawRectangle(pl.x * TILE, pl.y * TILE, TILE, TILE, SKYBLUE);
            DrawCircle(en.x * TILE + TILE / 2, en.y * TILE + TILE / 2, TILE / 2 - 4, RED);

            DrawText(TextFormat("Level: %d", level), 10, 10, 20, YELLOW);
            DrawText(TextFormat("Time: %.1f sec", frameCount / 30.0), 10, 35, 20, YELLOW);
            DrawText(TextFormat("High Score: %d", highScore), 10, 60, 20, YELLOW);
        }

        else if (gameState == WIN) {
            const char* winText = "Level Complete!";
            const char* nextPrompt = "Press ENTER for Next Level";
            int winW = MeasureText(winText, 30);
            int promptW = MeasureText(nextPrompt, 20);
            DrawText(winText, (W * TILE - winW) / 2, TILE * 7, 30, GREEN);
            DrawText(nextPrompt, (W * TILE - promptW) / 2, TILE * 9, 20, GRAY);

            if (IsKeyPressed(KEY_ENTER)) {
                PlaySound(sMenu);
                level++;
                InitLevel();
                gameState = PLAYING;
            }
        }

        else if (gameState == GAMEOVER) {
            const char* overText = "GAME OVER";
            const char* restart = "Press R to Restart";
            int overW = MeasureText(overText, 40);
            int restartW = MeasureText(restart, 20);
            DrawText(overText, (W * TILE - overW) / 2, TILE * 6, 40, RED);
            DrawText(restart, (W * TILE - restartW) / 2, TILE * 8, 20, DARKGRAY);

            if (IsKeyPressed(KEY_R)) {
                PlaySound(sMenu);
                level = 1;
                InitLevel();
                gameState = PLAYING;
            }
        }

        EndDrawing();
    }

    UnloadSound(sWin);
    UnloadSound(sOver);
    UnloadSound(sMenu);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
