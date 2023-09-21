/* main.c: defines main() for cc1, cc1plus, etc.
   Copyright (C) 2007-2019 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "diagnostic-core.h"
#include "toplev.h"

int main (int argc, char **argv);

/* %%% RML1 debug log and pass-ordering code, in ordf_init() called by main */
#define RML_WHICHMAIN "main.c"
#include "dbf1.c"

#include <stdlib.h>
#include "ord1.c"

/* We define main() to call toplev::main(), which is defined in toplev.c.
   We do this in a separate file in order to allow the language front-end
   to define a different main(), if it so desires.  */

int
main (int argc, char **argv)
{
  int rv;
  toplev toplev (NULL, /* external_timer */
		 true /* init_signals */);

  ordf_init("main.c");
  fprintf(rml_dbf1, "main.c after ordf_init()\n");
  fprintf(rml_dbf1, "args:\n");
  fprintf(rml_rpt2, "# args:\n");
  for(int i=0; i<argc; i++) {
    fprintf(rml_dbf1, "  %2d %s\n", i, (const char *)(argv[i]));
    fprintf(rml_rpt2, "#   %2d %s\n", i, (const char *)(argv[i]));
  }
  {
    const char * rml_p1_pathname;
    rml_p1_pathname = getenv("RML1_ORD_PATH");
    if (rml_p1_pathname) {
      fprintf(rml_dbf1, "rml.p1_pathname == '%s'\n", rml_p1_pathname);
    }
  }
  fflush(rml_dbf1);

  rv = toplev.main (argc, argv);

  ordf_end();
  dbf1_end();

  return rv;
}
