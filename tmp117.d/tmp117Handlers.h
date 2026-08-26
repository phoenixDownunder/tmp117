/* tmp117Handlers.h */

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
 * tmp117Handlers.h - The routines to read and manage the tmp117 temperature sensor
 * Amended April 2023.
 * Amended August 2026.
 *
 * The specific device parameters with the initial i2c addresses of the
 * device are set in i2cDevicesHeader.h
 *
 * Once the argument settings are set, the temperature reading continues the routine exits.
 * Any parameters set using the CLAs are retained in the tmp117 until restart or reset.
 * The options are not saved to shared memory as per other devices.
 * Exit on any fail.
 */

#ifndef TMP117_HANDLERS_H
#define TMP117_HANDLERS_H

#define HIGH_TEMP_ALERT_MASK 0x8000
#define LOW_TEMP_ALERT_MASK  0x4000
#define DATA_READY_MASK      0x2000
#define SET_CONVERSION_MASK  0x0C00
#define SET_AVERAGINGE_MASK  0x0060

#define useconds_t unsigned int

int g_quit = 0;

struct timespec ts; /* Unused in the end. */

/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- 
 * Main procedures.
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */

int tmp117Prepare(int _debug);
int tmp117Loop(int fp, int _g_loop, int _debug);

/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- 
 * Internal procedures.
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */

void   intHandler(int sig);
void   displayTMP117Registers(int fp);
void   tmp117PrintHelp();
int    tmp117WaitForDeviceReadStatus(int fp, int _debug);
double tmp117ReadTemperature(int fp, int _debug);
int    tmp117ReadChipID(int fp, int _debug);

/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- 
 * .c
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */

void intHandler(int sig) {
   printf("\nInterrupt: Exiting.\n");
   g_quit = 1;
   return;
}

/*
 * Display all tmp117 registers
 * Address Range : 0x00 - 0x08 and 0x0F
 */
void displayTMP117Registers(int fp) {
   __u16 buffer[16] = {0};
   int index;
   int maxAddress = 0x08;
   char * regs[] = {"Temp_Result   Temperature Result Register",
                    "Configuration Configuration Register",
                    "THigh_Limit   Temperature High Limit Register",
                    "TLow_Limit    Temperature Low  Limit Register",
                    "EEPROM_UL     EEPROM Unlock Register",
                    "EEPROM1       EEPROM1 Register",
                    "EEPROM2       EEPROM2 Register",
                    "Temp_Offset   Temperature Offset Register",
                    "EEPROM3       EEPROM3 Register NIST required",
                    "", "", "", "","","",
                    "Device_ID     Device ID Register" };
   
   if (fp <= 0) return; /* Just in case. */

   for (index = 0; index <= maxAddress; index++ )
      buffer[index] = i2cWordRead(fp, index, FLIP);
   index = 0x0F;
   buffer[index] = i2cWordRead(fp, index, FLIP);

   printf("All TMP117 Registers\n");
   for (index = 0; index <= maxAddress; index++ )
      printf("0x%02X : 0x%04X : %s\n", index, buffer[index], regs[index]);
   index = 0x0F;
      printf("0x%02X : 0x%04X : %s\n", index, buffer[index], regs[index]);
   return;
}

/*
 * Help details for the tmp117 ? 
 */
void tmp117PrintHelp() {
   char * processName = strrchr(getenv("_"), '/'); /* argv[0] */
   processName++;

   printf("\n");
   printf("%s [debug] [i2c adapterAddress] [slaveAddress] ", processName);
   printf("[-d] [-M 0xXXXX | YYY] [-m 0xXXXX | YYY] ");
   printf("-o [YYY] -c [1 | c] -a [0,8,32,64] -v]\n");
   printf("Argument options for the tmp117 are...\n");
   printf("?, -?, -h, -H, h, H: Print Help and close.\n");
   printf("-d : set default values for all device registers\n");
   printf("-o : to set Temperature Offset Register [0..256] (may take a read cycle to clear the temperature buffer)\n");
   printf("-c : to set one shot conversion [1] or continuous conversion [c or C] (Set but unused here)\n");
   printf("-a : set the number for averaged rates of conversion [0,8(default),32,64]\n");
   printf("-v : Display all device registers\n");
   printf("-M : for Max or set High Limit Temperature Register 0xXXXX or [+-][0..256]\n");
   printf("-m : for Min or set Low  Limit Temperature Register 0xXXXX or [+-][0..256]\n");
   printf("-n : set loop count [0 for infinite or nn seconds]\n");
   printf("-f : Display the OUTPUT temperature in Fahrenheit\n");
   printf("-A : Set Temperature (1) or Alert (0) Mode\n");
   printf("-p : set Alert pin - DR Data Ready (1) or Alert pin (0)\n"); 
   printf("-P : set Alert Polarity Active High (1) or low (0)\n");
   printf("\n");
   return;
}

/*
 * Wait for the ready status.
 * ~2.5 seconds or thereabouts.
 * Hardwired.
 * Exit on timeout failure as device will have gone offline.
 * Amended: commented out all the waiting indicators.
 */
int tmp117WaitForDeviceReadStatus(int fp, int _debug) {
   __u8  statusRegister = 0x01;
   __u16 ustatus = 0x00;
   int counter = 0;
   if (fp <= 0) { /* This shouldn't never happen, just in case. */
      errno = ENXIO; /* ENXIO (6) No such device or address */
      perror("Error: Device is not ready, exiting.");
      exit(0);
   }
   if (_debug) { printf("."); fflush(stdout); }
   ustatus = i2cWordRead(fp, statusRegister, FLIP);
   while ((ustatus & 0x0200) != 0x0200) {
      mDelay(50); /* 50mS */
      if (_debug) { printf("."); fflush(stdout); }
      if ( ((++counter) > 50) ) {
         return -1;
      }
      ustatus = i2cWordRead(fp, statusRegister, FLIP);
   }
   if (_debug) printf("\n");
   return 0;
}

double tmp117ReadTemperature(int fp, int _debug) {
   __u16 value  = 0;
   int _status   = 0;
   double _aTemp = 0.0;
   if (fp <= 0) return -1; /* Technically could have an exact -1 degree temp. */

   _status = tmp117WaitForDeviceReadStatus(fp, 0); /* 0 == disable local _debug */
   if (_status < 0) {
      printf("Error: Device timeout, exiting.\n");
      return -1;
   }
   value = i2cWordRead(fp, 0x00, FLIP);
   _aTemp = ((double)(value) * 7.8125) / 1000.0;
   if (g_v) {
         printf("\n");
         if (convertToFahrenheit) {
            _aTemp = ( ((_aTemp * 9.0) / 5.0) + 32.0 );
            printf("Temperature is 0x%04X or %u Raw or %.6F %c%cF \n", value, value, _aTemp, 0xC2, 0xB0);
         }
      else
            printf("Temperature is 0x%04X or %u Raw or %.6F %c%cC \n", value, value, _aTemp, 0xC2, 0xB0);
   } else {
      if (convertToFahrenheit) {
            _aTemp = ( ((_aTemp * 9.0) / 5.0) + 32.0 );
            printf("%.2f\n", _aTemp);
      }
      else printf("%.2f\n", _aTemp);
   }
   if (_aTemp == -1.0) _aTemp = -1.001; /* never going to happen! */ 
   return _aTemp;
}

int tmp117ReadChipID(int fp, int _debug) {
   __u16 chipID = 0;
   if (fp <= 0) return -1;
   chipID = i2cWordRead(fp, g_SlaveIDReg, FLIP);

   if (chipID != g_SlaveID) {
      printf("ChipID at register 0x%02X is 0x%04X. ", g_SlaveIDReg, chipID);
      printf("The expected ChipID 0x%04X NOT found.\n", g_SlaveID);
      return -1;
   }
   if (_debug) {
      printf("\n");
      printf("ChipID at register 0x%02X is 0x%04X, expected ID is 0x%04X\n",
         g_SlaveIDReg, chipID, g_SlaveID);
   }
   return chipID;
}

int tmp117Prepare(int _debug) {
   int _fp = 0;

   if (_debug) {
      printf("\n");
      printf("DEBUG mode On\n");
      printf("\n");
      if(__i2cScanAdapter(g_Adapter, 0) < 0) exit(0);
      printf("\n");
      printf("Adapter Address is %d, ", g_Adapter);
      printf("Slave Address is 0x%02X\n", g_SlaveAddress);
   }

   _fp = i2cPrepareDevice(g_Adapter, g_SlaveAddress);
   if (_fp <= 0) {
      printf("Device not found\n");
      return -1;
   }
   if (g_SlaveID != tmp117ReadChipID(_fp, _debug)) {
      close(_fp);
      return -1;
   }

   if (_debug) displayTMP117Registers(_fp);
   if (_debug) printf("End of DEBUG info.\n");
   return _fp;
}

int tmp117Loop(int fp, int _g_loop, int _debug) {
   int gLoop = 1;
   double __attribute__((unused)) aT = 0; /* Could be saved but... */
   if (fp <= 0) return -1;
   if (_g_loop < 0) return -1;
   if (_g_loop > 100000) return -1; /* Use 0 and interrrupt. */
   gLoop = _g_loop;

   switch(_g_loop) {
      case 0 :
         for (;;) {
            aT = tmp117ReadTemperature(fp, _debug);
            if (g_quit) break;
            mDelay(1000); /* 1 second */
         } 
         break;
      case 1 : 
            aT = tmp117ReadTemperature(fp, _debug);
            break;
      default :
         while (gLoop-- > 0) {
            aT = tmp117ReadTemperature(fp, _debug);
            if (g_quit) break;
            mDelay(1000);
         } 
         break;
   } 
   if (_debug) printf("Last conversion: %.2f\n", aT);
   return 0;
}

#endif
