/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef NIMBLE_EXAMPLE_MINIMAL_SIMULATION_H
#define NIMBLE_EXAMPLE_MINIMAL_SIMULATION_H
#include "input.h"

struct ExampleGame;
struct Clog;

typedef struct ExamplePlayerInputWithParticipantInfo {
    uint8_t participantId;
    ExamplePlayerInput playerInput;
    ExamplePlayerEmptyInputType nimbleInputType;
} ExamplePlayerInputWithParticipantInfo;

void exampleSimulationTick(struct ExampleGame* game,
    const ExamplePlayerInputWithParticipantInfo* input, size_t playerCount, Clog* log);

#endif
