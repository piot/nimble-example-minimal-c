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
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_VERBOSE(&self->log, "preAuthoritativeTicks()")
}

void gameAppAuthoritativeTick(void* _self, const TransmuteInput* _input)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_VERBOSE(&self->log, "authoritativeTick()")

    ExamplePlayerInput input;

    const TransmuteParticipantInput* firstPlayer = &_input->participantInputs[0];
    switch (firstPlayer->inputType) {
    case TransmuteParticipantInputTypeNoInputInTime:
        CLOG_C_NOTICE(&self->log, "authoritativeTick(noInputInTime)")
        input.inputType = ExamplePlayerInputTypeForced;
        break;
    case TransmuteParticipantInputTypeWaitingForReconnect:
        CLOG_C_NOTICE(&self->log, "authoritativeTick(waitingForReconnect)")
        input.inputType = ExamplePlayerInputTypeWaitingForReconnect;
        break;
    case TransmuteParticipantInputTypeNormal:
        input.input.inGameInput = *(const ExamplePlayerInGameInput*)firstPlayer->input;
        input.inputType = ExamplePlayerInputTypeInGame;
    }

    exampleSimulationTick(&self->authoritativeGame, &input);
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
    const ExamplePlayerInput* input = (const ExamplePlayerInput*)_input->participantInputs[0].input;
    exampleSimulationTick(&self->predictedGame, input);
    self->predictedStepId++;
}

void gameAppPredictionPostPredictionTicks(void* _self)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_VERBOSE(&self->log, "PostPredictionTick()")
}
