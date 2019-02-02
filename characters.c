#include "characters.h"

#define CHAR(name, width, data) static const Character ch_##name = { width, data };

CHAR(0, 4,
  " XX "
  "X XX"
  "XX X"
  "X  X"
  " XX ");

CHAR(1, 2,
  " X"
  "XX"
  " X"
  " X"
  " X");

CHAR(2, 4,
  " XX "
  "X  X"
  "  X "
  " X  "
  "XXXX");

CHAR(3, 4,
  "XXX "
  "   X"
  " XX "
  "   X"
  "XXX ");

CHAR(4, 4,
  "X  X"
  "X  X"
  "XXXX"
  "   X"
  "   X");

CHAR(5, 4,
  "XXXX"
  "X   "
  "XXX "
  "   X"
  "XXX ");

CHAR(6, 4,
  " XX "
  "X   "
  "XXX "
  "X  X"
  " XX ");

CHAR(7, 4,
  "XXXX"
  "   X"
  "  X "
  "  X "
  "  X ");

CHAR(8, 4,
  " XX "
  "X  X"
  " XX "
  "X  X"
  " XX ");

CHAR(9, 4,
  " XX "
  "X  X"
  " XXX"
  "   X"
  " XX ");

CHAR(A, 4,
  " XX "
  "X  X"
  "XXXX"
  "X  X"
  "X  X");

CHAR(B, 4,
  "XXX "
  "X  X"
  "XXX "
  "X  X"
  "XXX ");

CHAR(C, 4,
  " XX "
  "X  X"
  "X   "
  "X  X"
  " XX ");

CHAR(D, 4,
  "XXX "
  "X  X"
  "X  X"
  "X  X"
  "XXX ");

CHAR(E, 4,
  "XXXX"
  "X   "
  "XXX "
  "X   "
  "XXXX");

CHAR(F, 4,
  "XXXX"
  "X   "
  "XXX "
  "X   "
  "X   ");

CHAR(G, 4,
  " XX "
  "X   "
  "X XX"
  "X  X"
  " XX ");

CHAR(H, 4,
  "X  X"
  "X  X"
  "XXXX"
  "X  X"
  "X  X");

CHAR(I, 1,
  "X"
  "X"
  "X"
  "X"
  "X");

CHAR(J, 4,
  "   X"
  "   X"
  "   X"
  "X  X"
  " XX ");

CHAR(K, 4,
  "X  X"
  "X X "
  "XX  "
  "X X "
  "X  X");

CHAR(L, 4,
  "X   "
  "X   "
  "X   "
  "X   "
  "XXXX");

CHAR(M, 4,
  "X  X"
  "XXXX"
  "X  X"
  "X  X"
  "X  X");

CHAR(N, 4,
  "X  X"
  "XX X"
  "X XX"
  "X  X"
  "X  X");

CHAR(O, 4,
  " XX "
  "X  X"
  "X  X"
  "X  X"
  " XX ");

CHAR(P, 4,
  "XXX "
  "X  X"
  "XXX "
  "X   "
  "X   ");

CHAR(Q, 4,
  " XX "
  "X  X"
  "X  X"
  "X XX"
  " XXX");

CHAR(R, 4,
  "XXX "
  "X  X"
  "XXX "
  "X X "
  "X  X");

CHAR(S, 4,
  " XXX"
  "X   "
  " XX "
  "   X"
  "XXX ");

CHAR(T, 3,
  "XXX"
  " X "
  " X "
  " X "
  " X ");

CHAR(U, 4,
  "X  X"
  "X  X"
  "X  X"
  "X  X"
  " XX ");

CHAR(V, 3,
  "X X"
  "X X"
  "X X"
  "X X"
  " X ");

CHAR(W, 4,
  "X  X"
  "X  X"
  "X  X"
  "XXXX"
  "X  X");

CHAR(X, 4,
  "X  X"
  "X  X"
  " XX "
  "X  X"
  "X  X");

CHAR(Y, 3,
  "X X"
  "X X"
  " X "
  " X "
  " X ");

CHAR(Z, 4,
  "XXXX"
  "   X"
  " XX "
  "X   "
  "XXXX");

CHAR(PERIOD, 1,
  " "
  " "
  " "
  " "
  "X");

CHAR(COLON, 1,
  " "
  "X"
  " "
  "X");

CHAR(HASH, 5,
  " X X "
  "XXXXX"
  " X X "
  "XXXXX"
  " X X ");

CHAR(SPACE, 2,
  "  ");

CHAR(INVALID, 4,
  "XXXX"
  "XXXX"
  "XXXX"
  "XXXX"
  "XXXX");



CHAR(B0, 7,
  "XXXXXXX"
  "XXXXXXX"
  "XX   XX"
  "XX   XX"
  "XX   XX"
  "XX   XX"
  "XX   XX"
  "XX   XX"
  "XX   XX"
  "XX   XX"
  "XXXXXXX"
  "XXXXXXX");

CHAR(B1, 7,
  "   XX  "
  "  XXX  "
  " XXXX  "
  "   XX  "
  "   XX  "
  "   XX  "
  "   XX  "
  "   XX  "
  "   XX  "
  "   XX  "
  " XXXXXX"
  " XXXXXX ") ;

CHAR(B2, 7,
  "XXXXXXX"
  "XXXXXXX"
  "     XX"
  "     XX"
  "     XX"
  "XXXXXXX"
  "XXXXXXX"
  "XX     "
  "XX     "
  "XX     "
  "XXXXXXX"
  "XXXXXXX");

CHAR(B3, 7,
  "XXXXXXX"
  "XXXXXXX"
  "     XX"
  "     XX"
  "     XX"
  "XXXXXXX"
  "XXXXXXX"
  "     XX"
  "     XX"
  "     XX"
  "XXXXXXX"
  "XXXXXXX");

CHAR(B4, 7,
  "XX   XX"
  "XX   XX"
  "XX   XX"
  "XX   XX"
  "XX   XX"
  "XXXXXXX"
  "XXXXXXX"
  "     XX"
  "     XX"
  "     XX"
  "     XX"
  "     XX");

CHAR(B5, 7,
  "XXXXXXX"
  "XXXXXXX"
  "XX     "
  "XX     "
  "XX     "
  "XXXXXXX"
  "XXXXXXX"
  "     XX"
  "     XX"
  "     XX"
  "XXXXXXX"
  "XXXXXXX");

CHAR(B6, 7,
  "XXXXXXX"
  "XXXXXXX"
  "XX     "
  "XX     "
  "XX     "
  "XXXXXXX"
  "XXXXXXX"
  "XX   XX"
  "XX   XX"
  "XX   XX"
  "XXXXXXX"
  "XXXXXXX");

CHAR(B7, 7,
  "XXXXXXX"
  "XXXXXXX"
  "     XX"
  "     XX"
  "    XX "
  "    XX "
  "   XX  "
  "   XX  "
  "  XX   "
  "  XX   "
  " XX    "
  " XX    ");

CHAR(B8, 7,
  "XXXXXXX"
  "XXXXXXX"
  "XX   XX"
  "XX   XX"
  "XX   XX"
  "XXXXXXX"
  "XXXXXXX"
  "XX   XX"
  "XX   XX"
  "XX   XX"
  "XXXXXXX"
  "XXXXXXX");

CHAR(B9, 7,
  "XXXXXXX"
  "XXXXXXX"
  "XX   XX"
  "XX   XX"
  "XX   XX"
  "XXXXXXX"
  "XXXXXXX"
  "     XX"
  "     XX"
  "     XX"
  "XXXXXXX"
  "XXXXXXX");


CHAR(PROCESSING, 13,
  "   XXXXXXXX  "
  "   X      X  "
  "   X      X  "
  "   X      X  "
  "XXXXXXXXXXXXX"
  "X           X"
  "X X         X"
  "X           X"
  "X XXXXXXXXX X"
  "XXX       XXX"
  "  X       X  "
  "  XXXXXXXXX  ");

CHAR(NEW, 13,
  "             "
  "             "
  "             "
  "             "
  "             "
  "             "
  "             "
  "             "
  "XX         XX"
  "XX         XX"
  "XXXXXXXXXXXXX"
  "XXXXXXXXXXXXX");

CHAR(DOWNLOADING, 13,
  "     XXX     "
  "     XXX     "
  "     XXX     "
  "     XXX     "
  "     XXX     "
  "   XXXXXXX   "
  "    XXXXX    "
  "     XXX     "
  "XX    X    XX"
  "XX         XX"
  "XXXXXXXXXXXXX"
  "XXXXXXXXXXXXX");


CHAR(PENDING, 12,
  "    XXXX    "
  "  XX    XX  "
  " X   X    X "
  " X   X    X "
  "X    X     X"
  "X    X     X"
  "X    XXXX  X"
  "X          X"
  " X        X "
  " X        X "
  "  XX    XX  "
  "    XXXX  ");

CHAR(ABORTED, 12,
  "     XX     "
  "     XX     "
  "    XXXX    "
  "    X  X    "
  "   XX  XX   "
  "   XX  XX   "
  "  XXX  XXX  "
  "  XXX  XXX  "
  " XXXXXXXXXX "
  " XXXX  XXXX "
  "XXXXX  XXXXX"
  "XXXXXXXXXXXX");

CHAR(COMPLETED, 12,
  "          XX"
  "         XXX"
  "        XXX "
  "       XXX  "
  "      XXX   "
  "      XXX   "
  " XX  XXX    "
  " XXX XXX    "
  "  XX XX     "
  "   XXX      "
  "   XXX      ");

CHAR(CANCELED, 12,
  "XX        XX"
  "XXX      XXX"
  " XXX    XXX "
  "  XXX  XXX  "
  "   XXXXXX   "
  "    XXXX    "
  "    XXXX    "
  "   XXXXXX   "
  "  XXX  XXX  "
  " XXX    XXX "
  "XXX      XXX"
  "XX        XX");

CHAR(HELD, 10,
  "  XXX  XXX"
  "  XXX  XXX"
  "  XXX  XXX"
  "  XXX  XXX"
  "  XXX  XXX"
  "  XXX  XXX"
  "  XXX  XXX"
  "  XXX  XXX"
  "  XXX  XXX"
  "  XXX  XXX"
  "  XXX  XXX"
  "  XXX  XXX");

static const Character* font_SMALL[] = {
  &ch_0, &ch_1, &ch_2, &ch_3, &ch_4, &ch_5, &ch_6, &ch_7, &ch_8, &ch_9,
  &ch_A, &ch_B, &ch_C, &ch_D, &ch_E, &ch_F, &ch_G, &ch_H, &ch_I, &ch_J,
  &ch_K, &ch_L, &ch_M, &ch_N, &ch_O, &ch_P, &ch_Q, &ch_R, &ch_S, &ch_T,
  &ch_U, &ch_V, &ch_W, &ch_X, &ch_Y, &ch_Z
};

static const Character* font_BIG[] = {
  &ch_B0, &ch_B1, &ch_B2, &ch_B3, &ch_B4, &ch_B5, &ch_B6, &ch_B7, &ch_B8, &ch_B9
};

const Character* getCharacter(char ch)
{
    if ('0' <= ch && ch <= '9')
        return font_SMALL[ch - '0'];
    if ('A' <= ch && ch <= 'Z')
        return font_SMALL[10 + ch - 'A'];
    if ('a' <= ch && ch <= 'z')
        return font_SMALL[10 + ch - 'a'];
    if (ch == ' ')
        return &ch_SPACE;
    if (ch == '.')
        return &ch_PERIOD;
    if (ch == ':')
        return &ch_COLON;
    if (ch == '#')
        return &ch_HASH;
    return &ch_INVALID;
}

const Character* getDigit(unsigned char nr)
{
  return font_BIG[nr];
}


const Character* getIcon(char nr)
{
  switch (nr) {
  case 'D':
      return &ch_DOWNLOADING;
  case 'E':
      return &ch_PENDING;
  case 'H':
      return &ch_HELD;
  case 'P':
      return &ch_PROCESSING;
  case 'S': // stopped
      return &ch_HELD;
  case 'N':
      return &ch_CANCELED;
  case 'A':
      return &ch_ABORTED;
  case 'C':
      return &ch_COMPLETED;
  default:
      return 0;
  }
}
