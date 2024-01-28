/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <clog/console.h>
#include <example/app.h>
#include <example/client.h>
#include <example/gamepad.h>
#include <example/host.h>
#include <example/input.h>
#include <example/render.h>
#include <example/sleep.h>

#include <imprint/default_setup.h>

 #define USE_RENDER

clog_config g_clog;

// TODO: Should not need to define this temp_str
char g_clog_temp_str[CLOG_TEMP_STR_SIZE];

static ExamplePlayerInput gamepadToPlayerInput(ExampleGamepad* gamepad)
{
    ExampleGamepadState gamepadState;

    exampleGamepadUpdate(gamepad);
    exampleGamepadRead(gamepad, &gamepadState);
    ExamplePlayerInGameInput ingameInput = {
        .horizontalAxis = (int8_t)gamepadState.horizontal,
        .verticalAxis = (int8_t)gamepadState.vertical,
        .wantsToJump = gamepadState.buttons != 0,
    };

    ExamplePlayerInput playerInput = {
        .input.inGameInput = ingameInput,
        .inputType = ExamplePlayerInputTypeInGame,
    };

    return playerInput;
}

#define NLR_MAX_LOCAL_PLAYERS (1)

static void addPredictedInput(ExampleClient* client, ExampleGamepad* gamepad)
{
    ExamplePlayerInput inputs[NLR_MAX_LOCAL_PLAYERS];
    uint8_t participantId[NLR_MAX_LOCAL_PLAYERS];
    TransmuteParticipantInput participantInputs[2];

    static const int useLocalPlayerCount = 1;

    for (size_t i = 0U; i < useLocalPlayerCount; ++i) {
        participantId[i] = client->nimbleEngineClient.nimbleClient.client.localParticipantLookup[i]
                               .participantId;
        inputs[i] = gamepadToPlayerInput(gamepad);
        participantInputs[i].input = &inputs[i];
        participantInputs[i].octetSize = sizeof(inputs[i]);
        participantInputs[i].participantId = participantId[i];
        participantInputs[i].inputType = TransmuteParticipantInputTypeNormal;
    }

    TransmuteInput transmuteInput;
    transmuteInput.participantInputs = participantInputs;
    transmuteInput.participantCount = useLocalPlayerCount;

    nimbleEngineClientAddPredictedInput(&client->nimbleEngineClient, &transmuteInput);
}

static void exampleClientAddInput(ExampleClient* client, ExampleGamepad* gamepad)
{
    transportStackSingleUpdate(&client->singleTransport);

    if (transportStackSingleIsConnected(&client->singleTransport)) {
        nimbleEngineClientUpdate(&client->nimbleEngineClient);
        if (client->nimbleEngineClient.phase == NimbleEngineClientPhaseSynced
            && client->nimbleEngineClient.nimbleClient.client.localParticipantCount > 0
            && nimbleEngineClientMustAddPredictedInput(&client->nimbleEngineClient)) {
            addPredictedInput(client, gamepad);
            /*
            if (app->frontend.phase == NlFrontendPhaseJoining) {
                app->frontend.phase = NlFrontendPhaseInGame;
                client->savedSecret
                    = client->nimbleEngineClient.nimbleClient.client.participantsConnectionSecret;
                client->hasSavedSecret = true;
            }
            */
        }

    } else {
        uint8_t buf[1200];
        datagramTransportReceive(&client->singleTransport.singleTransport, buf, 1200);
    }
}

int main(void)
{
    g_clog.log = clog_console;
    g_clog.level = CLOG_TYPE_VERBOSE;

    CLOG_INFO("Minimal Example Starts!")

    ImprintDefaultSetup imprintDefaultSetup;
    imprintDefaultSetupInit(&imprintDefaultSetup, 5 * 1024 * 1024);

    ImprintAllocator* allocator = &imprintDefaultSetup.tagAllocator.info;
    ImprintAllocatorWithFree* allocatorWithFree = &imprintDefaultSetup.slabAllocator.info;

    NimbleSerializeVersion applicationVersion = {
        1,
        0,
        0,
    };

    ExampleApp app;

    Clog gameAppClog = {
        .constantPrefix = "exampleGameApp",
        .config = &g_clog,
    };
    const StepId authoritativeStepId = { 0x8BADF00D };
    gameAppInit(&app.combinedGame, authoritativeStepId, gameAppClog);

    exampleGameInit(&app.combinedGame.authoritativeGame);
    exampleGameInit(&app.combinedGame.predictedGame);

    NimbleServerCallbackObjectVtbl serverCallbackKlass = {
        .authoritativeStateSerializeFn = gameAppAuthoritativeSerialize,
    };

    NimbleServerCallbackObject serverCallbackObject
        = { .vtbl = &serverCallbackKlass, .self = &app.combinedGame };

    exampleHostInit(&app.host, serverCallbackObject, app.combinedGame.authoritativeStepId,
        applicationVersion, allocator, allocatorWithFree);

    RectifyCallbackObjectVtbl callbackKlass = {
        .preAuthoritativeTicksFn = gameAppPreAuthoritativeTicks,
        .authoritativeTickFn = gameAppAuthoritativeTick,
        .authoritativeDeserializeFn = gameAppAuthoritativeDeserialize,
        .copyFromAuthoritativeToPredictionFn = gameAppCopyFromAuthoritativeToPrediction,
        .predictionTickFn = gameAppPredictionTick,
        .postPredictionTicksFn = gameAppPredictionPostPredictionTicks,
    };

    RectifyCallbackObject callbackObject = {
        .vtbl = &callbackKlass,
        .self = &app.combinedGame,
    };

    exampleClientInit( &app.client, callbackObject, applicationVersion, allocator, allocatorWithFree);

#if defined USE_RENDER
    ExampleRender render;
    exampleRenderInit(&render);
#endif
    ExampleGamepad gamepad;
    exampleGamepadInit(&gamepad);

    while (1) {

#if defined USE_RENDER
        exampleRenderUpdate(
            &render, &app.combinedGame.authoritativeGame, &app.combinedGame.predictedGame);
#endif
        exampleClientAddInput(&app.client, &gamepad);
        exampleClientUpdate(&app.client);
        exampleHostUpdate(&app.host);
        exampleSleepMs(16);
    }

#if defined USE_RENDER
    //exampleRenderClose(&render);
#endif

//    return 0;
}
