/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/nimble-example-minimal-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <example/sleep.h>
#include <time.h>
#include <clog/clog.h>
#include <errno.h>

void exampleSleepMs(size_t milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;

    int err = nanosleep(&ts, &ts);
    if (err != 0) {
       // CLOG_ERROR("NOT WORKING:%d", errno)
    }
}
