/* tmp117.c */

/*
BSD-3-Clause

Copyright (c) 2026 Bruce Stephens
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.

IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * tmp117.c is the Main routine to read the tmp117 temperature sensor.
 *
 * Amended April 2023.
 * Modified May 2025.
 * Amended August 2026.
 *
 * The specific device parameters with the initial i2c addresses of the
 * device are set in i2cDevicesHeader.h
 *
 * Once the argument settings are set, the temperature reading continues the routine exits.
 * Any parameters set using the CLAs are retained in the tmp117 until restart or reset.
 * The options are not saved to shared memory as per other devices.
 *
 * Exit on any fail.
 *
 * make all or compile using...
 * gcc -o tmp117 tmp117.c -I$HOME/headers/ -Wall -pedantic -std=c2x -D_GNU_SOURCE -li2c -lm
 */

#ifndef TMP117_C
#define TMP117_C

#define TMP117

#if __has_include("i2cIncludes.h")
#include "i2cIncludes.h"
#endif

#include "tmp117CLAHandlers.h"
#include "tmp117Handlers.h"

int main (int argc, char ** argv) {
   int fp = 0;

   setlocale(LC_ALL, "");
   signal(SIGINT, intHandler);

   tmp117SortCommandLineArguments(argc, argv);

   fp = tmp117Prepare(debug);   
   if (fp <= 0) exit(0);

   if (g_v && (!debug)) displayTMP117Registers(fp);

   tmp117Loop(fp, g_loop, debug);
   close(fp);
   exit(0);
}

#endif
