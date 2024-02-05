/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <example/app.h>
#include <example/input.h>
#include <example/simulation.h>
#include <mash/murmur.h>

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
    state->hash = mashMurmurHash3(state->gameState, state->gameStateOctetCount);
}

void gameAppAuthoritativeDeserialize(void* _self, const TransmuteState* state, StepId stepId)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_INFO(&self->log, "authoritativeDeserialize(%04X)", stepId)

    self->authoritative.game = *((const ExampleGame*)state->state);
    self->authoritative.stepId = stepId;
}

uint64_t gameAppAuthoritativeHash(void* _self)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_INFO(&self->log, "authoritativeHash")

    return mashMurmurHash3((const uint8_t*)&self->authoritative.game, sizeof(self->authoritative.game));
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

        examplePlayerInputWithParticipantInfo->participantId = participantInput->participantId;

        switch (participantInput->inputType) {
        case TransmuteParticipantInputTypeNoInputInTime:
            CLOG_C_NOTICE(log, "authoritativeTick(noInputInTime)")
            CLOG_ASSERT(participantInput->input == 0, "input should be null on NoInputInTime")
            tc_mem_clear_type(&examplePlayerInputWithParticipantInfo->playerInput);
            examplePlayerInputWithParticipantInfo->playerInput.inputType
                = ExamplePlayerInputTypeEmpty;
            examplePlayerInputWithParticipantInfo->nimbleInputType
                = ExamplePlayerEmptyInputTypeForced;
            break;
        case TransmuteParticipantInputTypeWaitingForReconnect:
            CLOG_C_NOTICE(log, "authoritativeTick(waitingForReconnect)")
            CLOG_ASSERT(participantInput->input == 0, "input should be null on WaitingForReconnect")
            tc_mem_clear_type(&examplePlayerInputWithParticipantInfo->playerInput);
            examplePlayerInputWithParticipantInfo->playerInput.inputType
                = ExamplePlayerInputTypeEmpty;
            examplePlayerInputWithParticipantInfo->nimbleInputType
                = ExamplePlayerEmptyInputTypeWaitingForReconnect;
            break;
        case TransmuteParticipantInputTypeNormal:
            CLOG_ASSERT(participantInput->input != 0,
                "input can not be null on TransmuteParticipantInputTypeNormal")
            examplePlayerInputWithParticipantInfo->playerInput
                = *(const ExamplePlayerInput*)participantInput->input;
            examplePlayerInputWithParticipantInfo->nimbleInputType
                = ExamplePlayerEmptyInputTypeNormal;
            break;
        }
    }

    exampleSimulationTick(&gameAndTickId->game, examplePlayerInputsWithParticipantInfos,
        _input->participantCount, log);
    gameAndTickId->stepId++;
}

void gameAppAuthoritativeTick(void* _self, const TransmuteInput* _input, StepId stepId)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_VERBOSE(&self->log, "authoritativeTick()")
    CLOG_ASSERT(stepId == self->authoritative.stepId, "predicted tick ID is wrong")

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

void gameAppPredictionTick(void* _self, const TransmuteInput* _input, StepId stepId)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_C_VERBOSE(&self->log, "PredictionTick()")
    CLOG_ASSERT(stepId == self->predicted.stepId, "predicted tick ID is wrong")
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
