#ifndef _UNSCRAMBLE_GAME_DATA_H_
#define _UNSCRAMBLE_GAME_DATA_H_

#include <QtCore>

struct unscrambleGameData
{
  // holds data for each word
  QString alphagram;
  quint8 numNotYetSolved;
  quint8 i, j; // indices in table
  QStringList solutions;
};

#endif
