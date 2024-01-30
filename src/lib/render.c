/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/game_app.h>
#include <example/render.h>
#include <ncurses.h>

void exampleRenderInit(ExampleRender* self)
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    self->numberOfRows = getmaxy(stdscr);
    self->numberOfColumns = getmaxx(stdscr);
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

static void convertPosition(ExampleRender* self, int x, int y, RenderPosition* outRenderPosition)
{
    outRenderPosition->x = self->xOffset + x;
    outRenderPosition->y = self->numberOfRows - 1 - y;
}

static void drawGameArea(ExampleRender* self)
{
    int width = 40;
    int height = 30;
    int lower = self->numberOfRows - 1;
    int left = self->xOffset;
    int upper = self->numberOfRows - 1 - height;
    int right = self->xOffset + width;

    mvhline(upper, left, '*', width);
    mvhline(lower, left, '*', width);
    mvvline(upper, left, '*', height);
    mvvline(upper, right, '*', height);
}

static void render(ExampleRender* self, ExampleGameAndStepId* gameAndStepId)
{
    ExampleGame* game = &gameAndStepId->game;
    drawGameArea(self);
    attron(COLOR_PAIR(1));

    RenderPosition renderPos;

    ExampleSnake* snake = &game->snake;

    convertPosition(self, snake->x[0], snake->y[0], &renderPos);
    mvprintw(renderPos.y, renderPos.x, "O");
    for (int i = 1; i < snake->length; i++) {
        convertPosition(self, snake->x[i], snake->y[i], &renderPos);
        mvprintw(renderPos.y, renderPos.x, "o");
    }

    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(2));
    convertPosition(self, game->food.x, game->food.y, &renderPos);
    mvprintw(renderPos.y, renderPos.x, "@");
    attroff(COLOR_PAIR(2));

    RenderPosition tickIdPos;

    convertPosition(self, 0, 40, &tickIdPos);

    mvprintw(tickIdPos.y, tickIdPos.x, "%04X", gameAndStepId->stepId);
}

void exampleRenderUpdate(ExampleRender* self, ExampleGameApp* combinedGame)
{
    (void)self;

    clear();

    self->xOffset = 0;
    render(self, &combinedGame->predicted);
    self->xOffset = 40;
    render(self, &combinedGame->authoritative);

    refresh();
}
