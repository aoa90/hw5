#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Dimensions for the drawn grid (should be GRIDSIZE * texture dimensions)
#define GRID_DRAW_WIDTH 640
#define GRID_DRAW_HEIGHT 640

#define WINDOW_WIDTH GRID_DRAW_WIDTH
#define WINDOW_HEIGHT (HEADER_HEIGHT + GRID_DRAW_HEIGHT)

// Header displays current score
#define HEADER_HEIGHT 50

// Number of cells vertically/horizontally in the grid
#define GRIDSIZE 10

typedef struct
{
    int x;
    int y;
} Position;

typedef enum
{
    TILE_GRASS,
    TILE_TOMATO
} TILETYPE;

TILETYPE grid[GRIDSIZE][GRIDSIZE];

Position playerPosition;
int score;
int level;
int numTomatoes;

bool shouldExit = false;

TTF_Font* font;

// get a random value in the range [0, 1]
double rand01();
void initGrid();
void initSDL();
void moveTo(int x, int y);
void handleKeyDown(SDL_KeyboardEvent* event);
void processInputs();
void drawGrid(SDL_Renderer* renderer, SDL_Texture* grassTexture, SDL_Texture* tomatoTexture, SDL_Texture* playerTexture);
void drawUI(SDL_Renderer* renderer);
int main(int argc, char* argv[]);