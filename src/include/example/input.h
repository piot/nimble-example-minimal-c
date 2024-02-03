/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef NIMBLE_EXAMPLE_MINIMAL_INPUT_H
#define NIMBLE_EXAMPLE_MINIMAL_INPUT_H

#include <stdint.h>
#include <stdbool.h>

/// First step to send for a joining participant
typedef struct ExamplePlayerSelectTeam {
    uint8_t preferredTeamToJoin;
} ExamplePlayerSelectTeam;

/// Used to control the avatar (snake)
typedef struct ExamplePlayerInGameInput {
    int8_t horizontalAxis;
    int8_t verticalAxis;
    bool abilityButton;
} ExamplePlayerInGameInput;

/// Defined which part of the ExamplePlayerInput input union to use
typedef enum ExamplePlayerInputType {
    ExamplePlayerInputTypeEmpty,
    ExamplePlayerInputTypeInGame,
    ExamplePlayerInputTypeSelectTeam,
} ExamplePlayerInputType;

/// Example Step
typedef struct ExamplePlayerInput {
    ExamplePlayerInputType inputType;
    union {
        ExamplePlayerInGameInput inGameInput;
        ExamplePlayerSelectTeam selectTeam;
    } input;
} ExamplePlayerInput;

#endif
