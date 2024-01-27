/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <clog/console.h>
#include <example/gamepad.h>
#include <example/host.h>
#include <example/input.h>
#include <example/render.h>
#include <example/simulation.h>
#include <example/sleep.h>

#include <imprint/default_setup.h>

#define USE_RENDER

clog_config g_clog;

// TODO: Should not need to define this temp_str
char g_clog_temp_str[CLOG_TEMP_STR_SIZE];

int main(void)
{
    g_clog.log = clog_console;
    g_clog.level = CLOG_TYPE_VERBOSE;

    CLOG_INFO("Minimal Example Starts!")

    ImprintDefaultSetup imprintDefaultSetup;
    imprintDefaultSetupInit(&imprintDefaultSetup, 5 * 1024 * 1024);

    ExampleHost host;

    ImprintAllocator* allocator = &imprintDefaultSetup.tagAllocator.info;
    ImprintAllocatorWithFree* allocatorWithFree = &imprintDefaultSetup.slabAllocator.info;

    NimbleSerializeVersion applicationVersion = {
        1,
        0,
        0,
    };

    ExampleGame authoritativeGame;
    exampleGameInit(&authoritativeGame);

    ExampleGame predictedGame;
    exampleGameInit(&predictedGame);

    ExamplePlayerInput playerInput = { .inputType = ExamplePlayerInputTypeInGame,
        .input.inGameInput.verticalAxis = -1,
        .input.inGameInput.horizontalAxis = -1 };

    exampleHostInit(&host, &applicationVersion, allocator, allocatorWithFree);

    exampleHostUpdate(&host);

#if defined USE_RENDER
    ExampleRender render;
    exampleRenderInit(&render);
#endif
    ExampleGamepad gamepad;
    exampleGamepadInit(&gamepad);

    ExampleGamepadState gamepadState;

    while (!authoritativeGame.gameIsOver) {
        exampleGamepadUpdate(&gamepad);
        exampleGamepadRead(&gamepad, &gamepadState);
        playerInput.input.inGameInput.horizontalAxis = (int8_t)gamepadState.horizontal;
        playerInput.input.inGameInput.verticalAxis = (int8_t)gamepadState.vertical;
        playerInput.input.inGameInput.wantsToJump = gamepadState.buttons != 0;

        exampleSimulationTick(&authoritativeGame, &playerInput);
        exampleSimulationTick(&predictedGame, &playerInput);

#if defined USE_RENDER
        exampleRenderUpdate(&render, &authoritativeGame, &predictedGame);
#endif
        exampleSleepMs(16);
    }

#if defined USE_RENDER
    exampleRenderClose(&render);
#endif

    return 0;
}
