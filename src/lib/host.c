/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/game.h>
#include <example/host.h>
#include <example/input.h>

void exampleHostInit(ExampleHost* self, const NimbleSerializeVersion* applicationVersion,
    ImprintAllocator* allocator, ImprintAllocatorWithFree* allocatorWithFree)
{
    Clog multiLog;
    multiLog.config = &g_clog;
    multiLog.constantPrefix = "multi";

    CLOG_INFO("initialize host multi transport")
    transportStackMultiInit(
        &self->multiTransport, allocator, allocatorWithFree, TransportStackModeLocalUdp, multiLog);

    transportStackMultiListen(&self->multiTransport, "localhost", 23000);

    const size_t maxConnectionCount = 4U;
    const size_t maxParticipantCount = 2;
    const size_t maxSingleParticipantStepOctetCount = sizeof(ExamplePlayerInput);

    Clog serverLog;
    serverLog.config = &g_clog;
    serverLog.constantPrefix = "NimbleServer";

    NimbleServerSetup serverSetup;
    serverSetup.maxSingleParticipantStepOctetCount = maxSingleParticipantStepOctetCount;
    serverSetup.maxParticipantCount = maxParticipantCount;
    serverSetup.maxConnectionCount = maxConnectionCount;
    serverSetup.maxParticipantCountForEachConnection = 1;
    serverSetup.maxWaitingForReconnectTicks = 62 * 20;
    serverSetup.maxGameStateOctetCount = sizeof(ExampleGame);
    serverSetup.memory = allocator;
    serverSetup.blobAllocator = allocatorWithFree;
    serverSetup.applicationVersion = *applicationVersion;
    serverSetup.now = monotonicTimeMsNow();
    serverSetup.log = serverLog;
    serverSetup.multiTransport = self->multiTransport.multiTransport;

    CLOG_INFO("initialize nimble server")

    int errorCode = nimbleServerInit(&self->nimbleServer, serverSetup);
    if (errorCode < 0) {
        CLOG_ERROR("could not initialize nimble server %d", errorCode)
        // return errorCode;
    }
    CLOG_INFO("nimble server is initialized")

    ExampleGame initialServerState;
    exampleGameInit(&initialServerState);

    // We just add a completely empty game. But it could be setup
    // with specific rules or game mode or similar
    // Since the whole game is blittable structs with no pointers, we can just cast it to an (uint8_t*)
    StepId stepId = 0xcafeU;
    nimbleServerReInitWithGame(&self->nimbleServer, (const uint8_t*)&initialServerState,
        sizeof(initialServerState), stepId, monotonicTimeMsNow());

    CLOG_INFO("nimble server has initial game state. octet count: %zu",
        self->nimbleServer.game.latestState.octetCount)
}

void exampleHostUpdate(ExampleHost* self)
{
    CLOG_VERBOSE("host update()")
    transportStackMultiUpdate(&self->multiTransport);
    nimbleServerUpdate(&self->nimbleServer, monotonicTimeMsNow());
}
