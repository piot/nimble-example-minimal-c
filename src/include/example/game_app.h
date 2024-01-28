/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef NIMBLE_EXAMPLE_MINIMAL_GAME_APP_H
#define NIMBLE_EXAMPLE_MINIMAL_GAME_APP_H

#include <example/game.h>
#include <nimble-server/serialized_game_state.h>

typedef struct ExampleGameApp {
    ExampleGame authoritativeGame;
    ExampleGame predictedGame;
    StepId authoritativeStepId;
    StepId predictedStepId;
    Clog log;
} ExampleGameApp;

void gameAppInit(ExampleGameApp* self, StepId stepId, Clog log);

void gameAppAuthoritativeSerialize(void* _self, NimbleServerSerializedGameState* state);
void gameAppAuthoritativeDeserialize(void* self, const TransmuteState* state);
void gameAppPreAuthoritativeTicks(void* self);
void gameAppAuthoritativeTick(void* self, const TransmuteInput* input);
void gameAppCopyFromAuthoritativeToPrediction(void* self, StepId tickId);
void gameAppPredictionTick(void* self, const TransmuteInput* input);
void gameAppPredictionPostPredictionTicks(void* self);

#endif