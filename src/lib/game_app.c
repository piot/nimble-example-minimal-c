/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <example/app.h>
#include <example/input.h>
#include <example/simulation.h>

void gameAppInit(ExampleGameApp* self, StepId authoritativeStepId, Clog log)
{
    self->log = log;
    self->authoritativeStepId = authoritativeStepId;
}

void gameAppAuthoritativeSerialize(void* _self, NimbleServerSerializedGameState* state)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_INFO(&self->log, "authoritativeSerialize()")

    state->gameState = (const uint8_t*)&self->authoritativeGame;
    state->stepId = self->authoritativeStepId;
    state->gameStateOctetCount = sizeof(self->authoritativeGame);
}

void gameAppAuthoritativeDeserialize(void* _self, const TransmuteState* state)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_INFO(&self->log, "authoritativeDeserialize()")

    self->authoritativeGame = *((const ExampleGame*)state->state);
}

void gameAppPreAuthoritativeTicks(void* _self)
{
#if defined CLOG_ENABLED
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_VERBOSE(&self->log, "preAuthoritativeTicks()")
#else
    (void)_self;
#endif
}

static void gameAppTick(ExampleGame* game, const TransmuteInput* _input, Clog* log)
{
    ExamplePlayerInput input;
#if !defined CLOG_LOG_ENABLED
    (void) log;
#endif

    const TransmuteParticipantInput* firstPlayer = &_input->participantInputs[0];
    switch (firstPlayer->inputType) {
    case TransmuteParticipantInputTypeNoInputInTime:
        CLOG_C_NOTICE(log, "authoritativeTick(noInputInTime)")
        input.inputType = ExamplePlayerInputTypeForced;
        break;
    case TransmuteParticipantInputTypeWaitingForReconnect:
        CLOG_C_NOTICE(log, "authoritativeTick(waitingForReconnect)")
        input.inputType = ExamplePlayerInputTypeWaitingForReconnect;
        break;
    case TransmuteParticipantInputTypeNormal:
        input = *(const ExamplePlayerInput*)firstPlayer->input;
        input.inputType = ExamplePlayerInputTypeInGame;
    }

    exampleSimulationTick(game, &input);
}

void gameAppAuthoritativeTick(void* _self, const TransmuteInput* _input)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_VERBOSE(&self->log, "authoritativeTick()")

    gameAppTick(&self->authoritativeGame, _input, &self->log);
    self->authoritativeStepId++;
}

void gameAppCopyFromAuthoritativeToPrediction(void* _self, StepId tickId)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_INFO(&self->log, "CopyFromAuthoritative()")
    (void)tickId;
    CLOG_ASSERT(tickId == self->authoritativeStepId, "authoritative tick ID is wrong")
    self->predictedGame = self->authoritativeGame;
    self->predictedStepId = self->authoritativeStepId;
}

void gameAppPredictionTick(void* _self, const TransmuteInput* _input)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_VERBOSE(&self->log, "PredictionTick()")
    gameAppTick(&self->predictedGame, _input, &self->log);
    self->predictedStepId++;
}

void gameAppPredictionPostPredictionTicks(void* _self)
{
#if defined CLOG_ENABLED
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_VERBOSE(&self->log, "PostPredictionTick()")
#else
    (void)_self;
#endif
}
