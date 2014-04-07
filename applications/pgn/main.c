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
#include  "Parser.h"
#include  <ctype.h>

#define LNWIDTH  80
#define LNWIDTH2 40
#define LNMOVES  8

static void
PgnGame_Center(char * lnbuf, int center, size_t space, const char * fmt, ...)
{
    va_list ap;
    char buf[LNWIDTH + 1];

    va_start(ap, fmt);
    vsnprintf(buf, space + 1, fmt, ap);
    va_end(ap);
    memcpy(lnbuf + center - strlen(buf) / 2, buf, strlen(buf));
}

static void
PgnGame_ShowEx(PgnGame_t * game)
{
    int index, y;
    const PgnMove_t * move;
    char moves[LNMOVES][16];
    int materials[LNMOVES][2];
    char boards[LNMOVES][8][9];
    char lnbuf[LNWIDTH + 1];
    char movebuf[9];
    lnbuf[LNWIDTH] = 0;

    memset(lnbuf, '-', LNWIDTH);
    printf("%s\n", lnbuf);

    memset(lnbuf, ' ', LNWIDTH);
    PgnGame_Center(lnbuf, LNWIDTH2 - 1, LNWIDTH - 4, "Date: %s", game->Date);
    printf("%s\n", lnbuf);

    memset(lnbuf, ' ', LNWIDTH);
    PgnGame_Center(lnbuf, LNWIDTH2 - 1, 2, "vs");
    PgnGame_Center(lnbuf, LNWIDTH / 4, LNWIDTH2 - 3,
		   "%s(%d)", game->White, game->WhiteElo);
    PgnGame_Center(lnbuf, LNWIDTH * 3 / 4, LNWIDTH2 - 3,
		   "%s(%d)", game->Black, game->BlackElo);
    printf("%s\n", lnbuf);

    memset(lnbuf, ' ', LNWIDTH);
    PgnGame_Center(lnbuf, LNWIDTH2 - 1, LNWIDTH - 4,
		   "%s (%s)", game->resultInfo, game->Result);
    printf("%s\n", lnbuf);

    PgnGame_ToStart(game);
    while (!PgnGame_AtEnd(game)) {
	for (index = 0; index < LNMOVES; ++index) {
	    moves[index][0] = 0;
	    materials[index][0] = 0;
	    materials[index][1] = 0;
	    for (y = 0; y < 8; ++y)
		strcpy(boards[index][y], "        ");
	}
	for (index = 0; index < LNMOVES; ++index) {
	    if (PgnGame_AtEnd(game)) break;
	    move = PgnGame_GetMove(game);
	    if (move->upgrade == pgnBlank)
		snprintf(moves[index], sizeof(moves[index]), "%s%s",
			 move->value, move->annotation);
	    else
		snprintf(moves[index], sizeof(moves[index]), "%s=%c%s",
			 move->value, toupper(move->upgrade), move->annotation);
	    PgnGame_Forward(game);
	    materials[index][0] = game->status.white.material;
	    materials[index][1] = game->status.black.material;
	    for (y = 0; y < 8; ++y)
		strcpy(boards[index][y], game->status.board[y]);
	}
	for (index = 0; index < LNMOVES; ++index) {
	    memset(movebuf, ' ', 8); movebuf[8] = 0;
	    if (index % 2 != 0)
		memcpy(movebuf + (8 - strlen(moves[index])) / 2,
		       moves[index], strlen(moves[index]));
	    printf(" %8s ", movebuf);
	}
	printf("\n");
	for (y = 7; y >= 0; --y) {
	    for (index = 0; index < LNMOVES; ++index)
		printf(" %8s ", boards[index][y]);
	    printf("\n");
	}
	for (index = 0; index < LNMOVES; ++index) {
	    memset(movebuf, ' ', 8); movebuf[8] = 0;
	    if (index % 2 == 0)
		memcpy(movebuf + (8 - strlen(moves[index])) / 2,
		       moves[index], strlen(moves[index]));
	    printf(" %8s ", movebuf);
	}
	printf("\n");
	for (index = 0; index < LNMOVES; ++index) {
	    printf(" %2d vs %2d ", materials[index][0], materials[index][1]);
	}
	printf("\n");
    }

    memset(lnbuf, '-', LNWIDTH);
    printf("%s\n", lnbuf);
}

int
main(int argc, char * argv[])
{
    PgnParser_t parser;
    PgnGame_t * cur;

    if (argc != 2) {
	fprintf(stderr, "%s PGN-FILENAME\n", argv[0]);
	goto errquit0;
    }
    if (!strcmp(argv[1], "-")) {
	if (!PgnParser(&parser, stdin, stderr)) goto errquit0;
    } else {
	if (!PgnParser_ByName(&parser, argv[1], stderr)) goto errquit0;
    }
    PgnParser_Parse(&parser);
    for (cur = parser.firstGame; cur; cur = cur->next)
	PgnGame_ShowEx(cur);
    PgnParser_Destruct(&parser);
    return 0;
 errquit0:
    return -1;
}
