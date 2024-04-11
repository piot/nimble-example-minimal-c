/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef NIMBLE_EXAMPLE_MINIMAL_CLIENT_H
#define NIMBLE_EXAMPLE_MINIMAL_CLIENT_H

#include <clog/clog.h>
#include <nimble-engine-client/client.h>
#include <stdbool.h>
#include <transport-stack/single.h>

struct ImprintAllocator;
struct ImprintAllocatorWithFree;

typedef struct ExampleClient {
    TransportStackSingle singleTransport;
    NimbleEngineClient nimbleEngineClient;
    Clog log;
    bool hasSavedSecret;
    NimbleSerializeParticipantConnectionSecret savedSecret;
    bool autoJoinEnabled;
} ExampleClient;

void exampleClientInit(ExampleClient* self, RectifyCallbackObject callbackObject,
    NimbleSerializeVersion version, struct ImprintAllocator* allocator,
    struct ImprintAllocatorWithFree* allocatorWithFree, bool isServerClient, Clog log);
void exampleClientUpdate(ExampleClient* self);

#endif
