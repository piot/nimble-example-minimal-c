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
    *seed = *seed * a + c;
    return *seed;
}

static uint8_t spawnAvatar(ExampleGame* self, uint8_t playerIndex)
{
    uint8_t snakeIndex = self->snakes.snakeCount;
    ExampleSnake* snake = &self->snakes.snakes[snakeIndex];
    snake->body[0].x = 2;
    snake->body[0].y = 2;
    snake->length = 1;
    snake->controlledByPlayerIndex = playerIndex;
    self->snakes.snakeCount++;
    return snakeIndex;
}

static void handleInGameInput(
    ExampleGame* self, ExamplePlayer* player, const ExamplePlayerInput* playerInput)
{
    if (player->snakeIndex == EXAMPLE_ILLEGAL_INDEX) {
        CLOG_NOTICE("player tried to play without an avatar, ignoring")
        return;
    }
    //CLOG_ASSERT(player->snakeIndex != EXAMPLE_ILLEGAL_INDEX, "illegal snake index");
    ExampleSnake* snake = &self->snakes.snakes[player->snakeIndex];
    snake->movementDirection
        = directionFromInput(&playerInput->input.inGameInput, snake->movementDirection);
    //CLOG_VERBOSE("movement direction is %02X", snake->movementDirection)
}

static void spawnAvatarForPlayer(ExampleGame* self, ExamplePlayer* player)
{
    if (player->snakeIndex == EXAMPLE_ILLEGAL_INDEX) {
        CLOG_NOTICE("participant %hhu (player index %02X) selected team and no avatar ",
            player->assignedToParticipantIndex, player->playerIndex)
        player->snakeIndex = spawnAvatar(self, player->playerIndex);
        CLOG_NOTICE("participant %hhu (player index %02X) has avatar (snake) %hhu",
            player->assignedToParticipantIndex, player->playerIndex, player->snakeIndex)
    } else {
        CLOG_NOTICE(
            "participant %hhu (player index %02X) selected team. already have an avatar %hhu",
            player->assignedToParticipantIndex, player->playerIndex, player->snakeIndex)
    }
}

static void handleInput(
    ExampleGame* self, ExamplePlayer* player, const ExamplePlayerInput* playerInput)
{
    //CLOG_VERBOSE("handleInput type: %02X", playerInput->inputType)
    switch (playerInput->inputType) {
    case ExamplePlayerInputTypeSelectTeam: {
        spawnAvatarForPlayer(self, player);
    } break;
    case ExamplePlayerInputTypeInGame: {
        handleInGameInput(self, player, playerInput);
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

static ExamplePosition moduluPositionIfOnWall(ExampleGameArea area, ExamplePosition position)
{
    position.x = tc_modulo(position.x, (int)area.width);
    position.y = tc_modulo(position.y, (int)area.height);

    return position;
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

    nextPosition = moduluPositionIfOnWall(game->area, nextPosition);

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

    CLOG_NOTICE("spawning player for participant %hhu (player index %02X)",
        assignedPlayer->playerIndex, participantId)
    return assignedPlayer;
}

static ExamplePlayer* participantJoined(ExamplePlayers* players, ExampleParticipant* participant,
    uint8_t participantId, uint8_t partyId, Clog* log)
{
    (void)log;
    if (participant->isUsed) {
        CLOG_ERROR("participant %hhu already in use", participantId)
    }

    participant->isUsed = true;
    participant->participantId = participantId;
    participant->partyId = partyId;

    ExamplePlayer* player = spawnPlayer(players, participant->participantId);
    participant->playerIndex = player->playerIndex;

    CLOG_C_INFO(log,
        "participant has joined. player count is %hhu. created player index %d for participant %d",
        players->playerCount, player->playerIndex, participant->participantId)

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

void exampleSimulationTick(ExampleGame* game, const ExamplePlayerInputWithParticipantInfo* _input,
    const size_t inputCount, Clog* log)
{
    for (size_t i = 0; i < inputCount; ++i) {
        const ExamplePlayerInputWithParticipantInfo* combinedPlayerInput = &_input[i];
        ExampleParticipant* participant
            = &game->participantLookup[combinedPlayerInput->participantId];

//        CLOG_INFO("participant %hhu type: %d", combinedPlayerInput->participantId,
  //          combinedPlayerInput->nimbleInputType)

        participant->state = ExampleParticipantStateNormal;

        switch (combinedPlayerInput->nimbleInputType) {
        case ExamplePlayerEmptyInputTypeJoined: {
            CLOG_INFO("participant joined %hhu", combinedPlayerInput->participantId)
            participantJoined(&game->players, participant, combinedPlayerInput->participantId,
                combinedPlayerInput->partyId, log);
        } break;
        case ExamplePlayerEmptyInputTypeLeft:
            if (!participant->isUsed) {
                CLOG_ERROR("participant %hhu can not leave, participant was not in use",
                    _input->participantId)
            }
            participantLeft(
                &game->players, &game->snakes, game->participantLookup, participant, log);
            break;
        case ExamplePlayerEmptyInputTypeNormal:
            break;
        case ExamplePlayerEmptyInputTypeWaitingForReJoin:
            participant->state = ExampleParticipantStateWaitingForRejoin;
            break;
        case ExamplePlayerEmptyInputTypeForced:
            continue;
        }
        if (!participant->isUsed) {
            continue;
        }
        ExamplePlayer* player = &game->players.players[participant->playerIndex];

        handleInput(game, player, &combinedPlayerInput->playerInput);
    }

    if (game->ticksBetweenMoves != 0) {
        game->ticksBetweenMoves--;
        return;
    }

    game->ticksBetweenMoves = 6;

    if (game->gameIsOver) {
        return;
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
