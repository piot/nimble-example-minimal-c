/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <example/app.h>
#include <example/input.h>
#include <example/simulation.h>

void gameAppAuthoritativeDeserialize(void* _self, const TransmuteState* state)
{
    CLOG_INFO("authoritativeDeserialize()")
    ExampleGameApp* self = (ExampleGameApp*)_self;

    self->authoritativeGame = *((const ExampleGame*)state->state);
}

void gameAppPreAuthoritativeTicks(void* _self)
{
    //ExampleGameApp* self = (ExampleGameApp*)_self;
    (void)_self;
    CLOG_INFO("preAuthoritativeTicks()")
}

void gameAppAuthoritativeTick(void* _self, const TransmuteInput* _input)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_INFO("authoritativeTick()")

    const ExamplePlayerInput* input = (const ExamplePlayerInput*)_input->participantInputs[0].input;

    exampleSimulationTick(&self->authoritativeGame, input);
}

void gameAppCopyFromAuthoritativeToPrediction(void* _self, StepId tickId)
{
    ExampleGameApp* self = (ExampleGameApp*)_self;
    CLOG_INFO("CopyFromAuthoritative()")
    (void)tickId;
    self->predictedGame = self->authoritativeGame;
}

void gameAppPredictionTick(void* _self, const TransmuteInput* _input)
{
    CLOG_INFO("PredictionTick()")
    const ExamplePlayerInput* input = (const ExamplePlayerInput*)_input->participantInputs[0].input;
    ExampleGameApp* self = (ExampleGameApp*)_self;
    exampleSimulationTick(&self->predictedGame, input);
}

void gameAppPredictionPostPredictionTicks(void* _self)
{
    (void)_self;
    CLOG_INFO("PostPredictionTick()")
}
