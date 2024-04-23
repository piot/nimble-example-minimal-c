/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/game_app.h>
#include <example/render.h>
#include <ncurses.h>

static const short WAITING_FOR_REJOIN_COLOR = 16;

void exampleRenderInit(ExampleRender* self, ExampleGameArea gameArea)
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    start_color();
    attr_off(A_BOLD, NULL);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);

    init_pair(8, COLOR_YELLOW, COLOR_BLACK);
    init_pair(WAITING_FOR_REJOIN_COLOR, COLOR_WHITE, COLOR_BLACK);

    self->numberOfRows = getmaxy(stdscr);
    self->numberOfColumns = getmaxx(stdscr);
    self->gameArea = gameArea;
    self->xOffsetReset = (int)gameArea.width + 5;
    self->yOffset = 1;
}

void exampleRenderClose(ExampleRender* self)
{
    (void)self;

    endwin();
}

typedef struct RenderPosition {
    int x;
    int y;
} RenderPosition;

static void convertPosition(
    ExampleRender* self, ExamplePosition simulationPosition, RenderPosition* outRenderPosition)
{
    outRenderPosition->x = self->xOffset + simulationPosition.x;
    outRenderPosition->y = self->numberOfRows - 1 - (self->yOffset + simulationPosition.y);
}

static void drawGameArea(ExampleRender* self, const ExampleGameArea gameArea)
{
    int upper = self->numberOfRows - 1 - ((int)gameArea.height + 1);
    int lower = self->numberOfRows - 1;

    int left = self->xOffset - 1;
    int right = self->xOffset - 1 + (int)gameArea.width + 2 - 1;

    mvhline(upper, left, '*', gameArea.width + 2);
    mvhline(lower, left, '*', gameArea.width + 2);
    mvvline(upper, left, '*', gameArea.height + 2);
    mvvline(upper, right, '*', gameArea.height + 2);
}

static void renderSnake(ExampleRender* self, const ExampleSnake* snake)
{
    RenderPosition renderPos;
    convertPosition(self, snake->body[0], &renderPos);
    mvprintw(renderPos.y, renderPos.x, "O");
    for (int i = 1; i < snake->length; i++) {
        convertPosition(self, snake->body[i], &renderPos);
        mvprintw(renderPos.y, renderPos.x, "o");
    }
}

static void renderFood(ExampleRender* self, const ExampleFood* food)
{
    attron(COLOR_PAIR(8) | A_BOLD);
    RenderPosition renderPos;
    convertPosition(self, food->position, &renderPos);
    mvprintw(renderPos.y, renderPos.x, "@");
    attroff(COLOR_PAIR(8) | A_BOLD);
}

static void renderHud(ExampleRender* self, StepId stepId, uint32_t hash)
{
    int y = (int)self->gameArea.height + 1;
    RenderPosition tickIdPos;
    ExamplePosition tickIdLogicalPos = { 0, y };
    convertPosition(self, tickIdLogicalPos, &tickIdPos);
    mvprintw(tickIdPos.y, tickIdPos.x, "%04X", stepId);

    y++;

    ExamplePosition hashLogical = { 0, y };
    RenderPosition hashRenderPos;
    convertPosition(self, hashLogical, &hashRenderPos);
    mvprintw(hashRenderPos.y, hashRenderPos.x, "%04X", hash);
}

static void render(ExampleRender* self, const ExampleGameAndStepId* gameAndStepId, uint32_t hash)
{
    const ExampleGame* game = &gameAndStepId->game;
    drawGameArea(self, game->area);

    for (size_t i = 0; i < game->snakes.snakeCount; ++i) {
        const ExampleSnake* snake = &game->snakes.snakes[i];
        const ExamplePlayer* player = &game->players.players[snake->controlledByPlayerIndex];
        const ExampleParticipant* participant
            = &game->participantLookup[player->assignedToParticipantIndex];

        int colorIndex = player->playerIndex + 1;
        if (participant->state == ExampleParticipantStateWaitingForRejoin) {
            colorIndex = WAITING_FOR_REJOIN_COLOR;
        }

        attron(COLOR_PAIR(colorIndex));

        renderSnake(self, snake);

        attroff(COLOR_PAIR(colorIndex));
    }

    renderFood(self, &game->food);

    renderHud(self, gameAndStepId->stepId, hash);
}

void exampleRenderUpdate(ExampleRender* self, const ExampleGameApp* combinedGame, uint32_t hash)
{
    (void)self;

    clear();

    self->xOffset = 1;
    render(self, &combinedGame->predicted, 0);
    self->xOffset = self->xOffsetReset;
    render(self, &combinedGame->authoritative, hash);

    refresh();
}
