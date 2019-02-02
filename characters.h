#pragma once

typedef struct {
  unsigned char width;
  const char* data;
} Character;

const Character* getCharacter(char ch);
const Character* getDigit(unsigned char nr);
const Character* getIcon(char ch);
