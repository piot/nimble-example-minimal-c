/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/game.h>
#include <example/input.h>
#include <example/simulation.h>

void exampleSimulationTick(ExampleGame* game, ExamplePlayerInput* input)
{
    switch (input->inputType) {
    case ExamplePlayerInputTypeInGame:
        game->positionX += input->input.inGameInput.horizontalAxis;
        break;

    case ExamplePlayerInputTypeForced:
    case ExamplePlayerInputTypeWaitingForReconnect:
    case ExamplePlayerInputTypeNone:
    case ExamplePlayerInputTypeSelectTeam:
        break;
    }
}
