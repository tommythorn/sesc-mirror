/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2004 University of Illinois.

   Contributed by Paul Sack
                  Luis Ceze

This file is part of SESC.

SESC is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2, or (at your option) any later version.

SESC is    distributed in the  hope that  it will  be  useful, but  WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should  have received a copy of  the GNU General  Public License along with
SESC; see the file COPYING.  If not, write to the  Free Software Foundation, 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <sys/types.h>
#include <dirent.h>
#include "nanassert.h"
#include "TraceReader.h"

void TraceReader::openTrace(char* basename) {
  char filename[80];
  int  count;

  strcpy(filename, basename);
  strcat(filename, "/thread_001.tt6e");
  trace = fopen(filename, "r");

  if (trace == NULL)
    exit(-1);

  readPC();
}

VAddr TraceReader::getNextPC() {
  ulong oldPC = PC;

  readInst();
  if (isBranch()) {
    readPC();
    I(!isMemory());
    IS(address = count = 0);
  }
  else {
    PC += 4;

    if (isMemory()) {
      readAddress();
      I(!isBranch());
      IS(count = 0);
    }
    else if (isMemoryExtended()) {
      readCount();
    }
    else { /* arithmetic/regular inst */
      IS(address = count = 0);
    }

  }


  return oldPC;
}
