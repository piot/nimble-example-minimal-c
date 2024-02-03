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
    self->authoritative.stepId = authoritativeStepId;
}

void gameAppAuthoritativeSerialize(void* _self, NimbleServerSerializedGameState* state)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_INFO(&self->log, "authoritativeSerialize()")

    state->gameState = (const uint8_t*)&self->authoritative.game;
    state->stepId = self->authoritative.stepId;
    state->gameStateOctetCount = sizeof(self->authoritative.game);
}

void gameAppAuthoritativeDeserialize(void* _self, const TransmuteState* state, StepId stepId)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_INFO(&self->log, "authoritativeDeserialize(%04X)", stepId)

    self->authoritative.game = *((const ExampleGame*)state->state);
    self->authoritative.stepId = stepId;
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

static void gameAppTick(
    ExampleGameAndStepId* gameAndTickId, const TransmuteInput* _input, Clog* log)
{
    ExamplePlayerInputWithParticipantInfo
        examplePlayerInputsWithParticipantInfos[EXAMPLE_GAME_MAX_PLAYERS];
#if !defined CLOG_LOG_ENABLED
    (void)log;
#endif

    for (size_t i = 0; i < _input->participantCount; ++i) {
        const TransmuteParticipantInput* participantInput = &_input->participantInputs[i];
        ExamplePlayerInputWithParticipantInfo* examplePlayerInputWithParticipantInfo
            = &examplePlayerInputsWithParticipantInfos[i];

        examplePlayerInputWithParticipantInfo->playerInput
            = *(const ExamplePlayerInput*)participantInput->input;
        examplePlayerInputWithParticipantInfo->participantId = participantInput->participantId;

        switch (participantInput->inputType) {
        case TransmuteParticipantInputTypeNoInputInTime:
            CLOG_C_NOTICE(log, "authoritativeTick(noInputInTime)")
            examplePlayerInputWithParticipantInfo->nimbleInputType = ExamplePlayerEmptyInputTypeForced;
            break;
        case TransmuteParticipantInputTypeWaitingForReconnect:
            CLOG_C_NOTICE(log, "authoritativeTick(waitingForReconnect)")
            examplePlayerInputWithParticipantInfo->nimbleInputType = ExamplePlayerEmptyInputTypeWaitingForReconnect;
            break;
        case TransmuteParticipantInputTypeNormal:
            examplePlayerInputWithParticipantInfo->nimbleInputType = ExamplePlayerEmptyInputTypeNormal;
            break;
        }
    }

    exampleSimulationTick(&gameAndTickId->game, examplePlayerInputsWithParticipantInfos,
        _input->participantCount, log);
    gameAndTickId->stepId++;
}

void gameAppAuthoritativeTick(void* _self, const TransmuteInput* _input)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_VERBOSE(&self->log, "authoritativeTick()")

    gameAppTick(&self->authoritative, _input, &self->log);
}

void gameAppCopyFromAuthoritativeToPrediction(void* _self, StepId tickId)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_INFO(&self->log, "CopyFromAuthoritative(%04X) to predicted (%04X)", tickId,
        self->authoritative.stepId)
    (void)tickId;
    CLOG_ASSERT(tickId == self->authoritative.stepId, "authoritative tick ID is wrong")
    self->predicted = self->authoritative;
}

void gameAppPredictionTick(void* _self, const TransmuteInput* _input)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_VERBOSE(&self->log, "PredictionTick()")
    gameAppTick(&self->predicted, _input, &self->log);
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
