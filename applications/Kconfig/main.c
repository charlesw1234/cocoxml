/*-------------------------------------------------------------------------
  Author (C) 2008, Charles Wang <charlesw123456@gmail.com>

  This program is free software; you can redistribute it and/or modify it 
  under the terms of the GNU General Public License as published by the 
  Free Software Foundation; either version 2, or (at your option) any 
  later version.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
  for more details.

  You should have received a copy of the GNU General Public License along 
  with this program; if not, write to the Free Software Foundation, Inc., 
  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
-------------------------------------------------------------------------*/
#include  "desc/Parser.h"
#include  "file/Parser.h"
#include  "c/IncPathList.h"
#include  <limits.h>

int
main(int argc, char * argv[])
{
    KcParser_t kcparser;
    CfParser_t cfparser;
    char configdir[PATH_MAX];

    if (argc != 3) {
	fprintf(stderr, "%s Kconfig-FILENAME Config-FILENAME\n", argv[0]);
	goto errquit0;
    }

    CcsDirname(configdir, sizeof(configdir), argv[2]);
    if (!strcmp(argv[1], "-")) {
	if (!KcParser(&kcparser, stdin, stderr)) goto errquit0;
    } else {
	if (!KcParser_ByName(&kcparser, argv[1], stderr)) goto errquit0;
    }
    kcparser.incdirs = CcsIncPathListV(FALSE, FALSE, configdir, NULL);
    if (!strcmp(argv[2], "-")) {
	if (!CfParser(&cfparser, stdin, stderr)) goto errquit1;
    } else {
	if (!CfParser_ByName(&cfparser, argv[2], stderr)) goto errquit1;
    }
    KcParser_Parse(&kcparser);
    CfParser_Parse(&cfparser);

    CfParser_Destruct(&cfparser);
    KcParser_Destruct(&kcparser);
    return 0;
 errquit1:
    KcParser_Destruct(&kcparser);
 errquit0:
    return -1;
}
