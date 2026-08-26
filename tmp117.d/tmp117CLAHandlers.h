/* tmp117CLAHandlers.h */

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
 * tmp117CLAHandlers.h - The Command Line Argument Handlers for the tmp117 temperature sensor
 *
 * Argument options for the tmp117 are...
 * [debug] [i2c adapterAddress (1)] [device slaveAddress (0x48)] \
 * [-n -d -M -m -o [0xXXXX | YYY] -c [1 | c | C] -a [0,8,32,64] -v]
 * [-A [1 | 0] Temperature or Alert Mode
 * [-P [1 | 0] Alert Polarity Active High or low
 * Argument options are...
 * debug mode. Overrules all other arguments for debug summary.
 * -n (-l) sets loop mode, nn seconds
 * -M for Max or set temperature high limit register
 * -m for Min or set temperature low  limit register
 * -o to set temperature offset register
 * -c to set one shot conversion or continuous conversion (can set but unused here) 
 * -a to set the number for averaged Rates of conversion [0,8(default),32,64]
 * -f Display the OUTPUT temperature in Fahrenheit
 * -A Temperature or Alert Mode: 1 or 0
 * -p Alert pin select DR or Alert: 1 or 0
 * -P Alert Polarity Active: 1 or 0
 * All other options are ignored.
 * 
 * The parameters are retained in the tmp117 until restart or reset
 * Argument structure is [-x [value]]...
 * argv or argv[0] points to the first argument, the program name
 *
 * Exit on any fail.
 */

#ifndef TMP117_CLA_HANDLERS_H
#define TMP117_CLA_HANDLERS_H

int debug  = 0;
int g_v    = 0; /* Global show device registers */
int g_loop = 1; /* Global loop counter. */
int convertToFahrenheit = 0;

/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- 
 * Internal prototypes.
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */

void   tmp117PrintHelp(); /* In tmp117Handlers.h. */
int    tmp117CheckStrArgument(char * s);
int    getAdapterAddressArgument(char * s);
void   noSlaveAddressExit();
int    getSlaveAddressArgument(char * s);
void   checkArgc(int ac, int _saveargc, char ** av);
int    tmp117SortCommandLineArguments(int argc, char ** argv);

/* --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- 
 * .c
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- */

/*
 * The checkStrArgument is used to check the validity of the values
 * passed as arguments for the options for the tmp117.
 */
int tmp117CheckStrArgument(char * s) {
   int ok = 0;
   int i  = 0;

   switch(strlen(s)) {
   case 0 :
      printf("%s : Error, NULL string, must be 0xX[XXX] or decimal [+-]Y[YY]\n", s);
      exit(0);
      break;
   case 1 :
      if ( (s[0] >= '0') & (s[0] <= '9') ) return (strtol(s, NULL, 10) * 128);
      break;
   case 2 :
      if ( ( (s[0] == '+') | (s[0] == '-') | ( (s[0] >= '0') & (s[0] <= '9') ) ) &
           ( (s[1] >= '0') & (s[1] <= '9') ) ) return (strtol(s, NULL, 10) * 128);
      break;
   case 3:
   case 4:
   case 5:
      if ( (s[0] == '0') & (toupper(s[1]) == 'X') ) /* Hex mode */
      {
         ok = 1;
         for (i = 2; i < strlen(s); i++)
            if ( ( (s[i] >= '0') & (s[i] <= '9') ) | ( ( (toupper(s[i]) >= 'A') & (toupper(s[i]) <= 'F') ) ) )
            {}
            else ok = 0;    
         if (ok) return strtol(s, NULL, 16);
      }
/* Decimal mode - restrict length to 3 after +-
 * Restrict the decimal options to +-256. Hex mode is unrestricted.
 */
      if ( (s[0] == '+') | (s[0] == '-') | ( (s[0] >= '0') & (s[0] <= '9') ) )
      {
         if ( ((s[0] == '+') | (s[0] == '-')) & (strlen(s) > 4) ) break;
         if ( ((s[0] >= '0') & (s[0] <= '9')) & (strlen(s) > 3) ) break;
         ok = 1;
         for (i = 1; i < strlen(s); i++)
           if ( (s[i] >= '0') | (s[i] <= '9') )
           {}
           else ok = 0;
         if (ok) {
           i = strtol(s, NULL, 10);
           if ( (i >= -256) & (i <= 256) ) return (i * 128);
           else {
                printf("%s : is out of range [-256..256]\n", s);
                exit(0);
                }
           }
      }
      break;
/*
 * Hex mode
 */
   case 6:
      if ( (s[0] == '0') & (toupper(s[1]) == 'X') )
      {
         ok = 1;
         for (i = 2; i < strlen(s); i++)
            if ( ((s[i] >= '0') & (s[i] <= '9')) | ((toupper(s[i]) >= 'A') & (toupper(s[i]) <= 'F')) )
            {}
            else ok = 0;    
         if (ok) return strtol(s, NULL, 16);
      }

   default:
      break;
   }
   printf("%s : Error, format must be 0xX[XXX] or decimal [+-]Y[YY]\n", s);
   exit(0);
}

/*
 * Recover and check the argv shell arguments.
 * The second MAIN argv if not an option is the slave address.
 * If it is different from the predeclared slave address,
 * set the global g_SlaveAddress.
 * Strictly speaking the protocol only allows for 0x08-0x7F...
 * BUT some devices can use 0x00... 
 * Return the slaveAddress on SUCCESS, exit on fail.
 * A short fail exit routine is used.
 */
int getAdapterAddressArgument(char * s) {
   long number = 0;

   if (strlen(s) > 2) {
      errno = EINVAL;
      perror("Error: adapter_number range [0-99] [excluding 20 and 21]");
      exit(0);
   }
   number = strtol(s, NULL, 10);
   if (number < 0 || number > 99 || number == 20 || number == 21) {
      errno = EINVAL;
      perror("Error: adapter_number range [0-99] [excluding 20 and 21]");
      exit(0);
   }
   g_Adapter = (__u8)number;
   return (int)number;
}

void noSlaveAddressExit() {
   errno = EINVAL;
   perror("Error: slave address range [[0x00 | 0x08] .. 0x7F]");
   exit(0);
}

int getSlaveAddressArgument(char * s) {
   long number = 0;
   errno = 0;
   if (strlen(s) > 4)                  noSlaveAddressExit();
   number = strtol(s, NULL, 16); 
   if (errno != 0)                     noSlaveAddressExit();
   if (number < 0x00 && number > 0x7F) noSlaveAddressExit();

   errno = 0;
   g_SlaveAddress = (__u8)number;
   return (int)number; 
}

/*
 * Checks argc and therefore pointers to argv are sensible
 */
void checkArgc(int ac, int _saveargc, char ** av) {
   if (ac <= 0) {
      printf("Invalid command line option : %s\n", *av);
      exit(0);
   }
}

/*
 * Command Line Options for the tmp117
 * Scan the Command Line Arguments and allocate them accordingly.
 * If there are no arguments, return 0 and just run the routine.
 * At this initial stage,
 * should be the adapter address followed by the slave Address.
 * OR options beginning with a hyphen -
 * POLICY:
 * If adapter is specified, must also specify slave address.
 * At this stage, the adapter address range is from [0-99] but
 * realistically only 0 to about 3, not including 20-21.
 *
 * The Raspberry Pi 4 uses /dev/i2c-20 and 21 for its own purposes.
 * The Apple MacBook (specifically an ancient 2008 13" model)
 * uses adapters /dev/i2c-0 through to /dev/i2c-9.
 *
 * The slaveAddress range is 0x08-0x7F so could cheat here.
 * If there is no adapter AND no slave address...
 * then scan for - options...
 * Or nothing further.
 */
int tmp117SortCommandLineArguments(int argc, char ** argv) {
   int fp = 0; /* Device file pointer  */
   int ad = 0; /* Possible new adapter */
   int sl = 0; /* Possible new slave   */
/* adsl is a variable indicating that a valid adapter and slave address have been found */
   int adsl = 0;

/* Use s = *++argv for convenience... if ( (*++argv)[0] == '-' )... */
   char * s;

/* Replacements for argc and argv */
   int ac = argc;
   int saveargc = argc;
   char ** pv;
   pv = argv;
   s = *pv;

   __u8  reg     = 0x00;
   __u16 value   = 0;
   __u16 ustatus = 0x00;
   int status    = 0;
   int offset    = 0;
   char * v;

   char * processName = strrchr(getenv("_"), '/'); /* Or argv[0] */
   processName++;

/* Close anything and start again clean */
   if (fp > 0) close(fp);
   fp = 0;

   debug = 0;

/* If there are no options, run the command with all current set or default options */
   if (ac == 1) return 0;

   while (--ac > 0) {
      s = *++pv;

      if ( tolower(s[0]) == 'h' ) { tmp117PrintHelp(); exit(0); }
      if ( s[0] == '?' ) { tmp117PrintHelp(); exit(0); }

/* Recover any argument and if it is exactly "DEBUG" then debug */
      if ( (strcmp(s,"DEBUG") == 0 ) | (strcmp(s,"debug") == 0) ) {
         debug = 1;
         break;
      }
/*
 * Here where we expect an updated adapter and slave address.
 * If and adapter and slave exist, must close any previous fp
 * Given a new adapter and slave address, test and open straight away...
 */
      if ( (adsl == 0) & (s[0] != '?') & (s[0] != '-') ) {
         ad = getAdapterAddressArgument(s);
         fp = __i2cOpenAdapter(ad);
         if (fp < 0) {
            errno = EINVAL;
            perror("Unknown adapter");
            exit(0);
         }
         g_Adapter = ad;

         if (--ac > 0) {
            s = *++pv;
            sl = getSlaveAddressArgument(s);
            if (__i2cSetSlaveDevice(fp, sl) < 0) {
               close(fp);
               errno = EINVAL;
               perror("IOCTL Slave not set");
               exit(0);
            }

            if (__i2cReadSlaveDeviceAddress(fp, g_SlaveAddress) < 0) {
               close(fp);
               errno = EINVAL;
               perror("Slave not found");
               exit(0);
            }
            g_SlaveAddress = sl;
            adsl = 1;
            if (--ac > 0) s = *++pv; else return 0;
         } else {
            errno = EINVAL;
            perror("No Slave Address");
            exit(0);
         }
      }

/*
 * At this stage, the adapter and slave addresses must be opened
 * as some of the option involve reading the device.
 * If the device is not open, no point in going any further.
 */
      if (fp <= 0) fp = i2cPrepareDevice(g_Adapter, g_SlaveAddress);
      if (fp <= 0) {
         errno = EINVAL;
         if (fp == -2) perror("Unknown Slave Device");
         else perror("Unknown Adapter ");
         exit(0);
      }

/*    if ( (*++argv or av)[0] == '-' ) ... */
      if ( (s[0] == '-') & (strlen(s) > 1) ) {
/* If this is true then look at the next character in the pv (argv) string */
         s = pv[0] + 1;
         {
/* options */
         switch (*s) {
         case '?' :
         case 'h' :
         case 'H' :
                    tmp117PrintHelp();
                    if (fp > 0) close(fp);
                    exit(0);
                    break; /* Ignored */
         case 'a' :
            --ac;
            checkArgc(ac, saveargc, pv);
            v = (*++pv); /* v is a pointer */
            printf("Conversion Averaging Modes... 0, 8, 32, 64]\n");
            if (strlen(v) > 2) {
               close(fp);
               printf("Error: %s: invalid option\n", v);
               exit(0);
            }
            reg = 0x01;
            ustatus = i2cWordRead(fp, reg, FLIP);
            printf("Configuration Register is 0x%04X\n", ustatus);

            if (*v == '0') ustatus =  ustatus & 0xFF9F;
               else
            if (*v == '8') ustatus = (ustatus & 0xFF9F) | 0x0020;
               else
            if ( (v[0] == '3') & (v[1] == '2') ) ustatus = (ustatus & 0xFF9F) | 0x0040;
               else
            if ( (v[0] == '6') & (v[1] == '4') ) ustatus = (ustatus & 0xFF9F) | 0x0060;
               else {
                  close(fp);
                  printf("%s: Error, invalid option\n", v);
                  exit(0);
               }
            printf("Configuration Register is 0x%04X\n", ustatus);
            status = i2cWordWrite(fp, reg, ustatus, FLIP);
/*
 * If these write commands fail, there will be perror message at the lower driver level
 */
            if (status != 0) { close(fp); exit(0); }
            break;

         case 'A' :
            --ac;
            checkArgc(ac, saveargc, pv);
            v = (*++pv); /* v is a pointer */
            printf("Thermal or Alert Mode... 1 or 0\n");
            if (strlen(v) > 1) {
               close(fp);
               printf("Error: %s: invalid option\n", v);
               exit(0);
            }
            reg = 0x01;
            ustatus = i2cWordRead(fp, reg, FLIP);
            printf("Configuration Register is 0x%04X\n", ustatus);

            if (*v == '1') ustatus = ustatus | 0x0010;
               else
            if (*v == '0') ustatus = ustatus & 0xFFEF;
               else {
                  close(fp);
                  printf("%s: Error, invalid option\n", v);
                  exit(0);
               }
            printf("Configuration Register is 0x%04X\n", ustatus);
            status = i2cWordWrite(fp, reg, ustatus, FLIP);
            if (status != 0) { close(fp); exit(0); }
            break;

         case 'c' :
            --ac;
            checkArgc(ac, saveargc, pv);
            v = (*++pv); /* v is a pointer */
            if (strlen(v) > 1) {
               printf("%s: %ld\n", v, strlen(v));
               exit(0);
            }
            reg = 0x01;
            ustatus = i2cWordRead(fp, reg, FLIP);
            printf("Configuration Register is 0x%04X\n", ustatus);
            if (*v == '1') ustatus = ustatus | 0x0C00;
               else
            if ((*v == 'c') || (*v == 'C')) ustatus = ustatus & 0xF3FF;
               else {
                  printf("%s: Error, invalid option\n", v);
                  ac = 0;
                  exit(0);
               }
            printf("Configuration Register is 0x%04X\n", ustatus);
            ustatus = i2cWordWrite(fp, reg, ustatus, FLIP);
            break;

         case 'd' :
         case 'D' :
/*
 * set defaults
 * 0x04:0x0000
 * 0x05:0x41F2
 * 0x06:0x0F69
 * 0x08:0x624B
 * 0x0F:0x0117 Read Only
 */
            reg = 0x01; value = 0x0220;
            status = i2cWordWrite(fp, reg, value, FLIP);
            reg = 0x02; value = 0x6000;
            status = i2cWordWrite(fp, reg, value, FLIP);
            reg = 0x03; value = 0x8000;
            status = i2cWordWrite(fp, reg, value, FLIP);
            reg = 0x07; value = 0x0000;
            status = i2cWordWrite(fp, reg, value, FLIP);
            ustatus = i2cWordRead(fp, reg, 1); /* Clears the Alerts */
            printf("Default registers set\n");
            if (status != 0) { close(fp); exit(0); } /* Yes cheating here */
            break;

         case 'f' :
         case 'F' :
             convertToFahrenheit = 1;
             break;

         case 'n' :
            --ac;
            checkArgc(ac, saveargc, pv);
            v = (*++pv);
            if (strlen(v) > 3) { printf("Loop count argument %s too long\n", v); close(fp); exit(0); }
            g_loop = 1;
            value = strtol(v, NULL, 10);
            if (value == 0) g_loop = 0;
               else g_loop = value;
            break;

         case 'm' :
            --ac;
            checkArgc(ac, saveargc, pv);
            v = (*++pv);
            value = tmp117CheckStrArgument(v);
            reg = 0x03;
            status = i2cWordWrite(fp, reg, value, FLIP);
            if (status != 0) { close(fp); exit(0); }
            printf("Temperature Low Limit Register set to 0x%02X (%d)\n", value, value);
            break;

         case 'M' :
            --ac;
            checkArgc(ac, saveargc, pv);
            v = (*++pv);
            value = tmp117CheckStrArgument(v);
            reg = 0x02;
            status = i2cWordWrite(fp, reg, value, FLIP);
            if (status != 0) { close(fp); exit(0); }
            printf("Temperature High Limit Register set to 0x%02X (%d)\n", value, value);
            break;

         case 'o' :
            --ac;
            checkArgc(ac, saveargc, pv);
            v = (*++pv);
            value  = 0;
            offset = 0;
            if (strlen(v) <= 4) offset = strtol(v, NULL, 10);
            if ((offset > -257) & (offset < 257)) value = (__u16) (offset * 128);
            reg = 0x07;
            status = i2cWordWrite(fp, reg, value, FLIP);
            if (status != 0) { close(fp); exit(0); }
            printf("Temperature Offset Register set to 0x%04X (%d)\n", value, offset);
            printf("It may take at least 1 read cycle to clear the temperature buffer.\n");
            break;

         case 'p' : 
            --ac;   
            checkArgc(ac, saveargc, pv);
            v = (*++pv);
            printf("Alert pin select DR flag (1) or Alert pin (0)...\n");
            if (strlen(v) > 1) {
               close(fp);
               printf("Error: %s: invalid option\n", v);
               exit(0);
            }
            reg = 0x01;
            ustatus = i2cWordRead(fp, reg, FLIP);
            printf("Configuration Register is 0x%04X\n", ustatus);

            if (*v == '1') ustatus = ustatus | 0x0004;
               else
            if (*v == '0') ustatus = ustatus & 0xFFFB;
               else {
                  close(fp);
                  printf("%s: Error, invalid option\n", v);
                  exit(0);
               }
            printf("Configuration Register is 0x%04X\n", ustatus);
            status = i2cWordWrite(fp, reg, ustatus, FLIP);
            if (status != 0) { close(fp); exit(0); }
            break;

         case 'P' : 
            --ac;   
            checkArgc(ac, saveargc, pv);
            v = (*++pv);
            printf("Set Alert Polarity... 1 or 0\n");
            if (strlen(v) > 1) {
               close(fp);
               printf("Error: %s: invalid option\n", v);
               exit(0);
            }
            reg = 0x01;
            ustatus = i2cWordRead(fp, reg, FLIP);
            printf("Configuration Register is 0x%04X\n", ustatus);

            if (*v == '1') ustatus = ustatus | 0x0008;
               else
            if (*v == '0') ustatus = ustatus & 0xFFF7;
               else {
                  close(fp);
                  printf("%s: Error, invalid option\n", v);
                  exit(0);
               }
            printf("Configuration Register is 0x%04X\n", ustatus);
            status = i2cWordWrite(fp, reg, ustatus, FLIP);
            if (status != 0) { close(fp); exit(0); }
            break;

         case 'v' :
             g_v = 1;
             break;

         default:
            printf("%s: illegal option %c\n", pv[0], *s);
//          exit(0);
            break;
            }
         }
      }
      else printf("Argument %s ignored\n",*pv);
   }
   return 0;
}

#endif
