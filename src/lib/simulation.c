/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <example/game.h>
#include <example/input.h>
#include <example/simulation.h>

#define WIDTH 40
#define HEIGHT 30

static void snakeMoveWithFront(ExampleSnake* snake)
{
    for (int i = snake->length - 1; i > 0; i--) {
        snake->x[i] = snake->x[i - 1];
        snake->y[i] = snake->y[i - 1];
    }
}

static void snakeSetDirectionFromInput(ExampleSnake* snake, const ExamplePlayerInGameInput* pad)
{
    if (pad->horizontalAxis < 0) {
        snake->movementDirection = ExampleDirectionLeft;
    } else if (pad->horizontalAxis > 0) {
        snake->movementDirection = ExampleDirectionRight;
    } else if (pad->verticalAxis < 0) {
        snake->movementDirection = ExampleDirectionDown;
    } else if (pad->verticalAxis > 0) {
        snake->movementDirection = ExampleDirectionUp;
    }
}

static void snakeMoveFrontUsingDirection(ExampleSnake* snake)
{
    switch (snake->movementDirection) {
    case ExampleDirectionDown:
        snake->y[0]--;
        break;
    case ExampleDirectionUp:
        snake->y[0]++;
        break;
    case ExampleDirectionLeft:
        snake->x[0]--;
        break;
    case ExampleDirectionRight:
        snake->x[0]++;
        break;
    }
}

static uint32_t pseudoRandomNext(uint32_t* seed)
{
    const uint32_t c = 1013904223;
    const uint32_t a = 1664525;
    *seed = (*seed * a + c);
    return *seed;
}

static uint8_t spawnAvatar(ExampleGame* self)
{
    uint8_t snakeIndex = self->snakes.snakeCount;
    ExampleSnake* snake = &self->snakes.snakes[snakeIndex];
    snake->x[0] = 2;
    snake->y[0] = 2;
    snake->length = 1;
    self->snakes.snakeCount++;
    return snakeIndex;
}

/*
static uint8_t spawnPlayer(ExampleGame* self)
{
    (void)self;
    uint8_t playerIndex = self->playerCount;
    ExamplePlayer* player = &self->players[self->playerCount];

    self->playerCount++;

    return playerIndex;
}
*/

static void handleInGameInput(ExampleGame* self, const ExamplePlayerInput* playerInput)
{
    ExamplePlayer* player = &self->players.players[playerInput->participantId];
    if (player->snakeIndex == EXAMPLE_ILLEGAL_INDEX) {
        CLOG_NOTICE("player tried to play without an avatar, ignoring");
        return;
    }
    //CLOG_ASSERT(player->snakeIndex != EXAMPLE_ILLEGAL_INDEX, "illegal snake index");
    ExampleSnake* snake = &self->snakes.snakes[player->snakeIndex];
    snakeSetDirectionFromInput(snake, &playerInput->input.inGameInput);
}

static void spawnAvatarForPlayer(ExampleGame* self, ExamplePlayer* player)
{
    if (player->snakeIndex != EXAMPLE_ILLEGAL_INDEX) {
        player->snakeIndex = spawnAvatar(self);
    }
}

static void handleInput(ExampleGame* self, const ExamplePlayerInput* playerInput)
{
    CLOG_DEBUG("handleInput %02x", playerInput->participantId)
    switch (playerInput->inputType) {
    case ExamplePlayerInputTypeSelectTeam: {
        ExamplePlayer* player = &self->players.players[playerInput->participantId];
        spawnAvatarForPlayer(self, player);
    } break;
    case ExamplePlayerInputTypeInGame: {
        handleInGameInput(self, playerInput);
    }
    case ExamplePlayerInputTypeForced:
    case ExamplePlayerInputTypeWaitingForReconnect:
    case ExamplePlayerInputTypeNone:
        break;
    }
}

static void simulateSnake(ExampleGame* game, ExampleSnake* snake)
{
    // Check if eaten food
    if (snake->x[0] == game->food.x && snake->y[0] == game->food.y) {
        snake->length++;
        uint32_t randomX = pseudoRandomNext(&game->pseudoRandom);
        uint32_t randomY = pseudoRandomNext(&game->pseudoRandom);
        game->food.x = randomX % (WIDTH - 2) + 1;
        game->food.y = randomY % (HEIGHT - 2) + 1;
    }

    snakeMoveWithFront(snake);
    snakeMoveFrontUsingDirection(snake);

    for (int i = 1; i < snake->length; i++) {
        if (snake->x[i] == snake->x[0] && snake->y[i] == snake->y[0]) {
            game->gameIsOver = true;
            CLOG_INFO("ate yourself")
            return;
        }
    }

    if (snake->x[0] <= 0 || snake->x[0] >= WIDTH - 1 || snake->y[0] <= 0
        || snake->y[0] >= HEIGHT - 1) {
        game->gameIsOver = true;
        CLOG_INFO("collided with wall %d, %d", snake->x[0], snake->y[0])
        return;
    }
}

static void simulateSnakes(ExampleGame* game)
{
    for (size_t i = 0; i < game->snakes.snakeCount; ++i) {
        ExampleSnake* snake = &game->snakes.snakes[i];
        simulateSnake(game, snake);
    }
}

static ExamplePlayer* spawnPlayer(ExamplePlayers* players, uint8_t participantId)
{
    size_t playerIndex = players->playerCount++;
    ExamplePlayer* assignedPlayer = &players->players[playerIndex];
    assignedPlayer->playerIndex = (uint8_t)playerIndex;
    assignedPlayer->assignedToParticipantIndex = participantId;
    assignedPlayer->snakeIndex = EXAMPLE_ILLEGAL_INDEX;
    // assignedPlayer->preferredTeamId = NL_TEAM_UNDEFINED;

    return assignedPlayer;
}

static ExamplePlayer* participantJoined(
    ExamplePlayers* players, ExampleParticipant* participant, Clog* log)
{
    (void)log;

    ExamplePlayer* player = spawnPlayer(players, participant->participantId);
    CLOG_C_INFO(log,
        "participant has joined. player count is %hhu. created player %d for participant %d",
        players->playerCount, player->playerIndex, participant->participantId)

    participant->playerIndex = player->playerIndex;
    participant->isUsed = true;

    return player;
}

static void removePlayer(
    ExampleParticipant* participants, ExamplePlayers* self, size_t indexToRemove)
{
    self->players[indexToRemove] = self->players[--self->playerCount];
    participants[self->players[indexToRemove].assignedToParticipantIndex].playerIndex
        = (uint8_t)indexToRemove;
}

static void despawnAvatar(ExamplePlayers* players, ExampleSnakes* avatars, size_t indexToRemove)
{
    CLOG_ASSERT(indexToRemove < avatars->snakeCount, "avatar index is corrupt")
    ExampleSnake* avatarToRemove = &avatars->snakes[indexToRemove];
    if (avatarToRemove->controlledByPlayerIndex != 0xff) {
        players->players[avatarToRemove->controlledByPlayerIndex].snakeIndex
            = EXAMPLE_ILLEGAL_INDEX;
    }

    avatars->snakes[indexToRemove] = avatars->snakes[--avatars->snakeCount];
}

static void participantLeft(ExamplePlayers* players, ExampleSnakes* avatars,
    ExampleParticipant* participants, ExampleParticipant* participant, Clog* log)
{
    (void)log;
    ExamplePlayer* assignedPlayer = &players->players[participant->playerIndex];
    int assignedAvatarIndex = assignedPlayer->snakeIndex;
    if (assignedAvatarIndex != EXAMPLE_ILLEGAL_INDEX) {
        despawnAvatar(players, avatars, (size_t)assignedAvatarIndex);
    }

    removePlayer(participants, players, participant->playerIndex);

    CLOG_C_INFO(log, "someone has left releasing player %hhu previously assigned to participant %d",
        participant->playerIndex, participant->participantId)

    participant->isUsed = false;
}

static void checkInputDiff(ExampleGame* self, const ExamplePlayerInputWithParticipantInfo* inputs,
    size_t inputCount, Clog* log)
{
    if (inputCount != self->lastParticipantLookupCount) {
        CLOG_C_INFO(log,
            "a participant has either been added or removed, count is different. was %hhu and is "
            "now %zu",
            self->lastParticipantLookupCount, inputCount)
    }

    for (size_t i = 0; i < EXAMPLE_GAME_MAX_PARTICIPANTS; ++i) {
        self->participantLookup[i].internalMarked = false;
    }

    for (size_t i = 0; i < inputCount; ++i) {
        ExampleParticipant* participant = &self->participantLookup[inputs[i].participantId];
        if (!participant->isUsed) {
            participant->isUsed = true;
            participant->participantId = inputs[i].participantId;
            ExamplePlayer* player = participantJoined(&self->players, participant, log);
            (void)player;
            // gameRulesForJoiningPlayer(self, player);
        }
        if (participant->playerIndex != 0xff) {
            self->players.players[participant->playerIndex].playerInput = inputs[i].playerInput;
        }
        participant->internalMarked = true;
    }

    for (size_t i = 0; i < EXAMPLE_GAME_MAX_PARTICIPANTS; ++i) {
        ExampleParticipant* participant = &self->participantLookup[i];
        if (participant->isUsed && !participant->internalMarked) {
            // An active participants that is no longer in the provided inputs must be removed
            participantLeft(
                &self->players, &self->snakes, self->participantLookup, participant, log);
        }
    }

    self->lastParticipantLookupCount = (uint8_t)inputCount;
}

void exampleSimulationTick(ExampleGame* game, const ExamplePlayerInputWithParticipantInfo* _input,
    const size_t inputCount, Clog* log)
{
    if (game->ticksBetweenMoves != 0) {
        game->ticksBetweenMoves--;
        return;
    }

    game->ticksBetweenMoves = 3;

    if (game->gameIsOver) {
        return;
    }

    checkInputDiff(game, _input, inputCount, log);

    for (size_t i = 0; i < inputCount; ++i) {
        const ExamplePlayerInputWithParticipantInfo* playerInput = &_input[i];
        handleInput(game, &playerInput->playerInput);
    }

    simulateSnakes(game);
}

const ExamplePlayer* exampleGameFindSimulationPlayerFromParticipantId(
    const ExampleGame* self, uint8_t participantId)
{
    for (size_t i = 0; i < self->players.playerCount; ++i) {
        const ExamplePlayer* simulationPlayer = &self->players.players[i];
        if (simulationPlayer->assignedToParticipantIndex == participantId) {
            return simulationPlayer;
        }
    }
    return 0;
}
