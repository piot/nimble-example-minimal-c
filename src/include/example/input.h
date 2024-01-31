/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef NIMBLE_EXAMPLE_MINIMAL_INPUT_H
#define NIMBLE_EXAMPLE_MINIMAL_INPUT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct ExamplePlayerInGameInput {
    int8_t horizontalAxis;
    int8_t verticalAxis;
    bool wantsToJump;
} ExamplePlayerInGameInput;

typedef struct ExamplePlayerSelectTeam {
    uint8_t preferredTeamToJoin;
} ExamplePlayerSelectTeam;

typedef enum ExamplePlayerInputType {
    ExamplePlayerInputTypeForced,
    ExamplePlayerInputTypeWaitingForReconnect,
    ExamplePlayerInputTypeNone,
    ExamplePlayerInputTypeInGame,
    ExamplePlayerInputTypeSelectTeam,
} ExamplePlayerInputType;

typedef struct ExamplePlayerInput {
    ExamplePlayerInputType inputType;
    union {
        ExamplePlayerInGameInput inGameInput;
        ExamplePlayerSelectTeam selectTeam;
    } input;
    uint8_t participantId;
} ExamplePlayerInput;

#endif
