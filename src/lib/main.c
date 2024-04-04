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
#include <mash/murmur.h>

#define USE_RENDER

clog_config g_clog;

// TODO: Should not need to define this temp_str
char g_clog_temp_str[CLOG_TEMP_STR_SIZE];

#define EXAMPLE_GAME_MAX_LOCAL_PLAYERS (1)

static ExamplePlayerInput gamepadToPlayerInput(const ExampleGamepad* gamepad)
{
    ExampleGamepadState gamepadState;

    exampleGamepadRead(gamepad, &gamepadState);
    ExamplePlayerInGameInput ingameInput = {
        .horizontalAxis = (int8_t)gamepadState.horizontal,
        .verticalAxis = (int8_t)gamepadState.vertical,
        .abilityButton = gamepadState.buttons != 0,
    };

    ExamplePlayerInput playerInput = {
        .input.inGameInput = ingameInput,
        .inputType = ExamplePlayerInputTypeInGame,
    };

    return playerInput;
}

static ExamplePlayerInput constructPlayerInput(const ExampleGame* authoritative,
    const ExampleGamepad* gamepad, uint8_t participantId, bool autoJoin)
{
    //CLOG_NOTICE("constructing input for participant %02X", participantId)
    const ExamplePlayer* simulationPlayer
        = exampleGameFindSimulationPlayerFromParticipantId(authoritative, participantId);
    if (simulationPlayer == 0 || simulationPlayer->snakeIndex == EXAMPLE_ILLEGAL_INDEX) {

        if (!autoJoin) {
            ExamplePlayerInput playerInput = {
                .inputType = ExamplePlayerInputTypeEmpty,
            };
            return playerInput;
        }
        // We haven't joined yet, keep asking
        CLOG_DEBUG("we haven't joined yet, asking to join for participant %02X", participantId)
        ExamplePlayerInput playerInput = {
            .input.selectTeam.preferredTeamToJoin = 1,
            .inputType = ExamplePlayerInputTypeSelectTeam,
        };
        return playerInput;
    }

    //CLOG_NOTICE(
     //   "we are joined, compiling gamepad input to send for participant %02X", participantId)
    return gamepadToPlayerInput(gamepad);
}

static void createTransmuteInput(const ExampleClient* client, const ExampleGame* authoritative,
    const ExampleGamepad* gamepad, TransmuteInput* outputTransmuteInput)
{
    static const int useLocalPlayerCount = 1;

    static ExamplePlayerInput inputs[EXAMPLE_GAME_MAX_LOCAL_PLAYERS];
    static TransmuteParticipantInput participantInputs[EXAMPLE_GAME_MAX_LOCAL_PLAYERS];

    for (size_t i = 0U; i < useLocalPlayerCount; ++i) {
        uint8_t participantId
            = client->nimbleEngineClient.nimbleClient.client.localParticipantLookup[i]
                  .participantId;
        inputs[i]
            = constructPlayerInput(authoritative, gamepad, participantId, client->autoJoinEnabled);

        participantInputs[i].input = &inputs[i];
        participantInputs[i].octetSize = sizeof(inputs[i]);
        participantInputs[i].participantId = participantId;
        participantInputs[i].inputType = TransmuteParticipantInputTypeNormal;
    }

    outputTransmuteInput->participantInputs = participantInputs;
    outputTransmuteInput->participantCount = useLocalPlayerCount;
}

static void addGamePredictedInput(
    ExampleClient* client, const ExampleGame* authoritative, ExampleGamepad* gamepad)
{
    //CLOG_C_NOTICE(&client->log, "creating predicted input")
    TransmuteInput transmuteInput;
    createTransmuteInput(client, authoritative, gamepad, &transmuteInput);
    nimbleEngineClientAddPredictedInput(&client->nimbleEngineClient, &transmuteInput);
}

static void exampleClientAddInput(
    ExampleClient* client, ExampleGame* authoritative, ExampleGamepad* gamepad)
{
    transportStackSingleUpdate(&client->singleTransport);
    exampleGamepadUpdate(gamepad);

    if (transportStackSingleIsConnected(&client->singleTransport)) {
        nimbleEngineClientUpdate(&client->nimbleEngineClient);
        if (client->nimbleEngineClient.phase == NimbleEngineClientPhaseSynced
            && client->nimbleEngineClient.nimbleClient.client.localParticipantCount > 0
            && nimbleEngineClientMustAddPredictedInput(&client->nimbleEngineClient)) {

            addGamePredictedInput(client, authoritative, gamepad);
        }

    } else {
        uint8_t buf[1200];
        datagramTransportReceive(&client->singleTransport.singleTransport, buf, 1200);
    }
}

int main(int argc, char* argv[])
{
    g_clog.log = clog_console;
    g_clog.level = CLOG_TYPE_VERBOSE;

    CLOG_INFO("Minimal Example Starts!")

    bool shouldHost = argc > 1 && tc_str_equal(argv[1], "host");
    ExampleHost host;

    if (shouldHost) {
        CLOG_INFO("hosting")
    } else {
        CLOG_INFO("remote only")
    }

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

    exampleGameInit(&app.combinedGame.authoritative.game);
    exampleGameInit(&app.combinedGame.predicted.game);

    NimbleServerCallbackObjectVtbl serverCallbackKlass = {
        .authoritativeStateSerializeFn = gameAppAuthoritativeSerialize,
    };

    NimbleServerCallbackObject serverCallbackObject
        = { .vtbl = &serverCallbackKlass, .self = &app.combinedGame };

    if (shouldHost) {
        exampleHostInit(&host, serverCallbackObject, app.combinedGame.authoritative.stepId,
            applicationVersion, allocator, allocatorWithFree);
    }

    RectifyCallbackObjectVtbl callbackKlass = {
        .preAuthoritativeTicksFn = gameAppPreAuthoritativeTicks,
        .authoritativeTickFn = gameAppAuthoritativeTick,
        .authoritativeDeserializeFn = gameAppAuthoritativeDeserialize,
        .authoritativeHashFn = gameAppAuthoritativeHash,
        .copyFromAuthoritativeToPredictionFn = gameAppCopyFromAuthoritativeToPrediction,
        .predictionTickFn = gameAppPredictionTick,
        .postPredictionTicksFn = gameAppPredictionPostPredictionTicks,
    };

    RectifyCallbackObject callbackObject = {
        .vtbl = &callbackKlass,
        .self = &app.combinedGame,
    };

    Clog clientAppClog = {
        .constantPrefix = "exampleClient",
        .config = &g_clog,
    };

    exampleClientInit(&app.client, callbackObject, applicationVersion, allocator, allocatorWithFree,
        clientAppClog);

    app.client.autoJoinEnabled = true; // shouldHost;

#if defined USE_RENDER
    ExampleRender render;
    exampleRenderInit(&render, app.combinedGame.authoritative.game.area);
#endif
    ExampleGamepad gamepad;
    exampleGamepadInit(&gamepad);

    while (1) {
#if defined USE_RENDER
        uint32_t hash = mashMurmurHash3(
            (const uint8_t*)&app.combinedGame.authoritative.game, sizeof(app.combinedGame.authoritative.game));
        exampleRenderUpdate(&render, &app.combinedGame, hash);
#endif
        exampleClientAddInput(&app.client, &app.combinedGame.authoritative.game, &gamepad);
        exampleClientUpdate(&app.client);
        if (shouldHost) {
            host.nimbleServer.game.debugIsFrozen = gamepad.internalState.debugPauseIsDown;
            exampleHostUpdate(&host);
        }
        exampleSleepMs(16);
    }

#if defined USE_RENDER
    //exampleRenderClose(&render);
#endif

    //    return 0;
}
