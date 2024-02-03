/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <example/game.h>
#include <example/input.h>
#include <example/simulation.h>

static void moveBodyOneStepIntoFront(ExampleSnake* snake)
{
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i].x = snake->body[i - 1].x;
        snake->body[i].y = snake->body[i - 1].y;
    }
}

static ExampleDirection directionFromInput(
    const ExamplePlayerInGameInput* pad, ExampleDirection defaultDirection)
{
    if (pad->horizontalAxis < 0) {
        return ExampleDirectionLeft;
    }
    if (pad->horizontalAxis > 0) {
        return ExampleDirectionRight;
    }
    if (pad->verticalAxis < 0) {
        return ExampleDirectionDown;
    }
    if (pad->verticalAxis > 0) {
        return ExampleDirectionUp;
    }

    return defaultDirection;
}

static bool positionsEqual(ExamplePosition a, ExamplePosition b)
{
    return a.x == b.x && a.y == b.y;
}

static ExamplePosition movePositionInDirection(
    ExamplePosition pos, ExampleDirection movementDirection)
{
    switch (movementDirection) {
    case ExampleDirectionDown:
        pos.y--;
        break;
    case ExampleDirectionUp:
        pos.y++;
        break;
    case ExampleDirectionLeft:
        pos.x--;
        break;
    case ExampleDirectionRight:
        pos.x++;
        break;
    }

    return pos;
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
    snake->body[0].x = 2;
    snake->body[0].y = 2;
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
    snake->movementDirection
        = directionFromInput(&playerInput->input.inGameInput, snake->movementDirection);
    CLOG_NOTICE("movement direction is %02X", snake->movementDirection)
}

static void spawnAvatarForPlayer(ExampleGame* self, ExamplePlayer* player)
{
    if (player->snakeIndex == EXAMPLE_ILLEGAL_INDEX) {
        CLOG_NOTICE("player %02X did not have a snacke, spawning it now", player->playerIndex)
        player->snakeIndex = spawnAvatar(self);
    } else {
        CLOG_NOTICE("player %02X already have snake, do not spawn", player->playerIndex)
    }
}

static void handleInput(
    ExampleGame* self, ExamplePlayer* player, const ExamplePlayerInput* playerInput)
{
    CLOG_DEBUG("handleInput %02x type: %02X", playerInput->participantId, playerInput->inputType)
    switch (playerInput->inputType) {
    case ExamplePlayerInputTypeSelectTeam: {
        spawnAvatarForPlayer(self, player);
    } break;
    case ExamplePlayerInputTypeInGame: {
        handleInGameInput(self, playerInput);
    } break;
    case ExamplePlayerInputTypeEmpty:
        break;
    }
}

static void spawnNewFood(ExampleGame* game)
{
    uint32_t randomX = pseudoRandomNext(&game->pseudoRandom);
    uint32_t randomY = pseudoRandomNext(&game->pseudoRandom);
    game->food.position.x = (int)(randomX % (game->area.width - 2) + 1);
    game->food.position.y = (int)(randomY % (game->area.height - 2) + 1);
}

static void checkIfSnakeAteFood(ExampleGame* game, ExampleSnake* snake)
{
    if (positionsEqual(snake->body[0], game->food.position)) {
        snake->length++;
        spawnNewFood(game);
    }
}

static bool checkIfCollidedWithSelf(ExampleSnake* snake, ExamplePosition checkPosition)
{
    for (int i = 1; i < snake->length; i++) {
        if (positionsEqual(snake->body[i], checkPosition)) {
            return true;
        }
    }

    return false;
}

static bool checkIfPositionIsOnWall(ExampleGameArea area, ExamplePosition position)
{
    if (position.x <= 0 || position.x >= (int)(area.width - 1) || position.y <= 0
        || position.y >= (int)(area.height - 1)) {
        return true;
    }

    return false;
}

static void simulateSnake(ExampleGame* game, ExampleSnake* snake)
{
    checkIfSnakeAteFood(game, snake);

    ExamplePosition nextPosition
        = movePositionInDirection(snake->body[0], snake->movementDirection);

    bool didCollide = checkIfCollidedWithSelf(snake, nextPosition);
    if (didCollide) {
        CLOG_NOTICE("collided with self")
        snake->isFrozen = true;
        return;
    }

    bool didCollideWithWall = checkIfPositionIsOnWall(game->area, nextPosition);
    if (didCollideWithWall) {
        CLOG_NOTICE("collided with wall")
        snake->isFrozen = true;
        return;
    }

    moveBodyOneStepIntoFront(snake);
    snake->body[0] = nextPosition;
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

    CLOG_NOTICE("spawning player: player index %02X for participant %02X", assignedPlayer->playerIndex, participantId)
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
            "a participant has either been added or removed, count is different. was %hhu and "
            "is "
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
            CLOG_NOTICE("we noticed a new participant with id %02X, lets spawn a player for it", inputs[i].participantId);
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
            CLOG_NOTICE("participant %02X left!?", participant->participantId)
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
        const ExamplePlayerInputWithParticipantInfo* combinedPlayerInput = &_input[i];
        ExampleParticipant* participant
            = &game->participantLookup[combinedPlayerInput->participantId];
        ExamplePlayer* player = &game->players.players[participant->playerIndex];

        handleInput(game, player, &combinedPlayerInput->playerInput);
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
