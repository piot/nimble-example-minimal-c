/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/game.h>
#include <example/host.h>
#include <example/input.h>

void exampleHostInit(ExampleHost* self, NimbleServerCallbackObject callbackObject,
    StepId authoritativeStepId, const NimbleSerializeVersion applicationVersion,
    ImprintAllocator* allocator, ImprintAllocatorWithFree* allocatorWithFree)
{
    Clog multiLog;
    multiLog.config = &g_clog;
    multiLog.constantPrefix = "multi";

    CLOG_INFO("initialize host multi transport")
    transportStackMultiInit(
        &self->multiTransport, allocator, allocatorWithFree, TransportStackModeLocalUdp, multiLog);

    transportStackMultiListen(&self->multiTransport, "127.0.0.1", 23000);

    const size_t maxConnectionCount = 4U;
    const size_t maxParticipantCount = 2;
    const size_t maxSingleParticipantStepOctetCount = sizeof(ExamplePlayerInput);

    Clog serverLog;
    serverLog.config = &g_clog;
    serverLog.constantPrefix = "NimbleServer";

    const NimbleServerSetup serverSetup
        = { .maxSingleParticipantStepOctetCount = maxSingleParticipantStepOctetCount,
              .maxParticipantCount = maxParticipantCount,
              .maxConnectionCount = maxConnectionCount,
              .maxParticipantCountForEachConnection = 1,
              .maxWaitingForReconnectTicks = 62 * 20,
              .maxGameStateOctetCount = sizeof(ExampleGame),
              .callbackObject = callbackObject,
              .memory = allocator,
              .targetTickTimeMs = 24,
              .blobAllocator = allocatorWithFree,
              .applicationVersion = applicationVersion,
              .now = monotonicTimeMsNow(),
              .log = serverLog,
              .multiTransport = self->multiTransport.multiTransport };

    CLOG_INFO("initialize nimble server")

    const int errorCode = nimbleServerInit(&self->nimbleServer, serverSetup);
    if (errorCode < 0) {
        CLOG_ERROR("could not initialize nimble server %d", errorCode)
        // return errorCode;
    }
    CLOG_INFO("nimble server is initialized")

    Clog exampleServerLog;
    exampleServerLog.config = &g_clog;
    exampleServerLog.constantPrefix = "exampleServer";

    self->log = exampleServerLog;

    // We just add a completely empty game. But it could be setup
    // with specific rules or game mode or similar
    // Since the whole game is blittable structs with no pointers, we can just cast it to an (uint8_t*)
    nimbleServerReInitWithGame(&self->nimbleServer, authoritativeStepId, monotonicTimeMsNow());
}

void exampleHostUpdate(ExampleHost* self)
{
    CLOG_C_VERBOSE(&self->log, "host update()")
    transportStackMultiUpdate(&self->multiTransport);
    nimbleServerUpdate(&self->nimbleServer, monotonicTimeMsNow());
}
