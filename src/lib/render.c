/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <curses.h>
#include <example/render.h>

void exampleRenderInit(ExampleRender* self)
{
    (void)self;
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
}

void exampleRenderClose(ExampleRender* self)
{
    (void)self;

    endwin();
}

static void render(ExampleGame* game, int yOffset)
{
    attron(COLOR_PAIR(1));
    mvprintw(game->positionY + yOffset, game->positionX, "@");
    attroff(COLOR_PAIR(1));
}

void exampleRenderUpdate(
    ExampleRender* self, ExampleGame* authoritativeGame, ExampleGame* predictedGame)
{
    (void)self;

    clear();

    render(authoritativeGame, 10);
    render(predictedGame, 20);

    refresh();
}
