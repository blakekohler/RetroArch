/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 * 
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

/* Test module to check re-entrancy of libretro implementations.
 * Reruns the program's main loop with all content defined on command-line
 * to check if libretro can load multiple content after each other.
 */

#include "../getopt_rarch.h"
#include "../general.h"
#include <string.h>

int rarch_main(int argc, char *argv[], void *data);

#undef main
int main(int argc, char *argv[])
{
   while (rarch_main(argc, argv, NULL) == 0)
   {
      if (optind + 1 >= argc)
         return 0;

      memmove(&argv[optind], &argv[optind + 1],
            (argc - optind - 1) * sizeof(char*));
      argc--;

      rarch_main_ctl(RARCH_MAIN_CTL_CLEAR_STATE);
   }
}

