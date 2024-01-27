/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <clog/console.h>
#include <example/host.h>
#include <example/input.h>
#include <example/render.h>
#include <example/simulation.h>
#include <example/sleep.h>
#include <example/gamepad.h>

#include <imprint/default_setup.h>

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
    authoritativeGame.positionX = 20;

    ExampleGame predictedGame;
    predictedGame.positionX = 40;

    ExamplePlayerInput playerInput
        = { .inputType = ExamplePlayerInputTypeInGame, .input.inGameInput.horizontalAxis = -1 };

    exampleHostInit(&host, &applicationVersion, allocator, allocatorWithFree);

    exampleHostUpdate(&host);

    ExampleRender render;

    exampleRenderInit(&render);

    ExampleGamepad gamepad;

    ExampleGamepadState gamepadState;

    for (int i = 0; i < 120; ++i) {
        exampleGamepadUpdate(&gamepad);
        exampleGamepadRead(&gamepad, &gamepadState);
        playerInput.input.inGameInput.horizontalAxis = (int8_t)gamepadState.horizontal;

        exampleSimulationTick(&authoritativeGame, &playerInput);
        exampleRenderUpdate(&render, &authoritativeGame, &predictedGame);
        exampleSleepMs(16);
    }

    exampleRenderClose(&render);

    return 0;
}
