/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef NIMBLE_EXAMPLE_MINIMAL_HOST_H
#define NIMBLE_EXAMPLE_MINIMAL_HOST_H

#include <nimble-server/server.h>
#include <transport-stack/multi.h>
#include <transport-stack/single.h>

typedef struct ExampleHost {
    NimbleServer nimbleServer;
    TransportStackMulti multiTransport;
    Clog log;
} ExampleHost;

void exampleHostInit(ExampleHost* self, const NimbleSerializeVersion* applicationVersion,
    ImprintAllocator* allocator, ImprintAllocatorWithFree* allocatorWithFree);
void exampleHostUpdate(ExampleHost* self);

#endif
