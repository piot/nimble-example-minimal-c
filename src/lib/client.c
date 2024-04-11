/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/client.h>
#include <example/input.h>
#include <imprint/allocator.h>

/// Initializes a nimble engine client on a previously setup single datagram transport
static void initNimbleEngineClient(NimbleEngineClient* self, RectifyCallbackObject callbackObject,
    DatagramTransport singleTransport, NimbleSerializeVersion applicationVersion,
    ImprintAllocator* allocator, ImprintAllocatorWithFree* allocatorWithFree)
{
    CLOG_DEBUG("start joining")

    CLOG_DEBUG("client datagram transport is set")

    NimbleEngineClientSetup setup = { .memory = allocator,
        .blobMemory = allocatorWithFree,
        .transport = singleTransport,
        .maximumSingleParticipantStepOctetCount = sizeof(ExamplePlayerInput),
        .maximumParticipantCount = 8,
        .applicationVersion = applicationVersion,
        .maxTicksFromAuthoritative = 20U,
        .rectifyCallbackObject = callbackObject,
        .wantsDebugStream = true };

    Clog nimbleEngineClientLog = { .config = &g_clog, .constantPrefix = "NimbleEngineClient" };

    setup.log = nimbleEngineClientLog;
    nimbleEngineClientInit(self, setup);

    CLOG_DEBUG("nimble client is setup with transport")
}

static void join(ExampleClient* self, bool isServerClient)
{
    static const int useLocalPlayerCount = 1;
    NimbleEngineClientGameJoinOptions joinOptions = { .playerCount = useLocalPlayerCount,
        .players[0].localIndex = 99,
        .players[1].localIndex = 42,
        .type = self->hasSavedSecret ? NimbleSerializeJoinGameTypeSecret : NimbleSerializeJoinGameTypeNoSecret,
        .secret = self->savedSecret };
    CLOG_DEBUG("nimble client is trying to join / rejoin server")

#if defined TEST_HOST_MIGRATION
    if (isServerClient) {
        joinOptions.type = NimbleSerializeJoinGameTypeHostMigrationParticipantId;
        joinOptions.participantId = 0x0a;
        CLOG_DEBUG("nimble client is trying to join after host migration as participant %hhu", joinOptions.participantId)
    }
#else
    (void) isServerClient;
#endif
    nimbleEngineClientRequestJoin(&self->nimbleEngineClient, joinOptions);
}

static void setupSingleTransport(TransportStackSingle* self, ImprintAllocator* allocator,
    ImprintAllocatorWithFree* allocatorWithFree)
{
    Clog singleLog;
    singleLog.config = &g_clog;
    singleLog.constantPrefix = "single";

    transportStackSingleInit(
        self, allocator, allocatorWithFree, TransportStackModeLocalUdp, singleLog);

    transportStackSingleConnect(self, "127.0.0.1", 23000);

    transportStackLowerLevelSetInternetSimulationMode(
        &self->lowerLevel, TransportStackInternetSimulationModeRecommended);
}

void exampleClientInit(ExampleClient* self, RectifyCallbackObject callbackObject,
    NimbleSerializeVersion version, ImprintAllocator* allocator,
    ImprintAllocatorWithFree* allocatorWithFree, bool isServerClient, Clog log)
{
    self->log = log;
    setupSingleTransport(&self->singleTransport, allocator, allocatorWithFree);
    initNimbleEngineClient(&self->nimbleEngineClient, callbackObject,
        self->singleTransport.singleTransport, version, allocator, allocatorWithFree);
    join(self, isServerClient);
}

void exampleClientUpdate(ExampleClient* self)
{
    transportStackSingleUpdate(&self->singleTransport);
    if (transportStackSingleIsConnected(&self->singleTransport)) {
        nimbleEngineClientUpdate(&self->nimbleEngineClient);
    } else {
        // This is just to make sure transport is settled
        uint8_t buf[DATAGRAM_TRANSPORT_MAX_SIZE];
        datagramTransportReceive(&self->singleTransport.singleTransport, buf, DATAGRAM_TRANSPORT_MAX_SIZE);
    }
}
