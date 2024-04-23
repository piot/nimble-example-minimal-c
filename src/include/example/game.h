/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef NIMBLE_EXAMPLE_MINIMAL_GAME_H
#define NIMBLE_EXAMPLE_MINIMAL_GAME_H

#include "input.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define NIMBLE_EXAMPLE_SNAKE_MAX_LENGTH (20)

#define EXAMPLE_ILLEGAL_INDEX (0xff)

typedef enum ExampleDirection {
    ExampleDirectionUp,
    ExampleDirectionRight,
    ExampleDirectionDown,
    ExampleDirectionLeft,
} ExampleDirection;

typedef struct ExamplePosition {
    int x;
    int y;
} ExamplePosition;

typedef struct ExampleGameArea {
    size_t width;
    size_t height;
} ExampleGameArea;

typedef struct ExampleSnake {
    ExamplePosition body[NIMBLE_EXAMPLE_SNAKE_MAX_LENGTH];
    int length;
    ExampleDirection movementDirection;
    uint8_t controlledByPlayerIndex;
    bool isFrozen;
} ExampleSnake;

typedef struct ExampleFood {
    ExamplePosition position;
} ExampleFood;

#define EXAMPLE_GAME_MAX_PLAYERS (4)
#define EXAMPLE_GAME_MAX_AVATARS (4)

typedef struct ExamplePlayer {
    uint8_t snakeIndex;
    uint8_t assignedToParticipantIndex;
    uint8_t playerIndex;
    ExamplePlayerInput playerInput;
} ExamplePlayer;

#define EXAMPLE_GAME_MAX_PARTICIPANTS (16)

typedef struct ExamplePlayers {
    ExamplePlayer players[EXAMPLE_GAME_MAX_PLAYERS];
    uint8_t playerCount;
} ExamplePlayers;

typedef enum ExampleParticipantState {
    ExampleParticipantStateNormal,
    ExampleParticipantStateWaitingForRejoin,
} ExampleParticipantState;

typedef struct ExampleParticipant {
    uint8_t participantId;
    uint8_t playerIndex;
    uint8_t partyId;
    bool isUsed;
    ExampleParticipantState state;
} ExampleParticipant;

typedef struct ExampleSnakes {
    ExampleSnake snakes[EXAMPLE_GAME_MAX_AVATARS];
    uint8_t snakeCount;
} ExampleSnakes;

typedef struct ExampleGame {
    ExampleGameArea area;
    ExamplePlayers players;
    ExampleSnakes snakes;
    ExampleFood food;
    uint32_t pseudoRandom;
    bool gameIsOver;
    uint32_t ticksBetweenMoves;

    uint8_t lastParticipantLookupCount;
    ExampleParticipant participantLookup[EXAMPLE_GAME_MAX_PARTICIPANTS];
} ExampleGame;

void exampleGameInit(ExampleGame* self);

const ExamplePlayer* exampleGameFindSimulationPlayerFromParticipantId(
    const ExampleGame* self, uint8_t participantId);

#endif
