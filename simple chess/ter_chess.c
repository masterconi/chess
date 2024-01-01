#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

#define BOARD_SIZE 8
#define SQUARE_SIZE 80

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int board[BOARD_SIZE][BOARD_SIZE] = {
    {2, 4, 3, 5, 6, 3, 4, 2},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {2, 4, 3, 5, 6, 3, 4, 2}
};

void renderText(SDL_Renderer* renderer, int x, int y, const char* text, SDL_Color color);
void renderBoardWithPieces();
int initializeSDL();
void handleEvents();

int main() {
    if (!initializeSDL()) {
        return 1;
    }

    while (1) {
        handleEvents();
        renderBoardWithPieces();
        SDL_Delay(100);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

int initializeSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 0;
    }

    window = SDL_CreateWindow("Chess", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return 0;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        return 0;
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf initialization failed: %s\n", TTF_GetError());
        return 0;
    }

    return 1;
}

void handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            SDL_Quit();
            exit(0);
        }
        // Add additional event handling code based on your requirements
    }
}

void renderText(SDL_Renderer* renderer, int x, int y, const char* text, SDL_Color color) {
   
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24);
    if (font == NULL) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect textRect = {x - surface->w / 2, y - surface->h / 2, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &textRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}

void renderBoardWithPieces() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            SDL_Rect squareRect = {col * SQUARE_SIZE, row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};

            if ((row + col) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 255, 206, 158, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 209, 139, 71, 255);
            }
            SDL_RenderFillRect(renderer, &squareRect);

            int piece = board[row][col];
            if (piece != 0) {
                char pieceText[2];
                snprintf(pieceText, sizeof(pieceText), "%d", piece);
                SDL_Color textColor = {0, 0, 0, 255};
                renderText(renderer, col * SQUARE_SIZE + SQUARE_SIZE / 2, row * SQUARE_SIZE + SQUARE_SIZE / 2, pieceText, textColor);
            }
        }
    }

    SDL_RenderPresent(renderer);
}
