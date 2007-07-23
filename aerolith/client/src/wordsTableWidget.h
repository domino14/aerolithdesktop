#ifndef _WORDS_TABLE_WIDGET_H_
#define _WORDS_TABLE_WIDGET_H_

#include <QtGui>

#define FIXED_ROWS 9
#define FIXED_COLS 5

class wordsTableWidget : public QTableWidget
{
 public:
  wordsTableWidget();
  void clearCells();
  QStringList getCellSolutions(int, int);
  QString getCellAlphagram(int, int);
  quint8 getCellNumSolutions(int, int);
  QBrush colorBrushes[9]; // add to utilities file
  void answeredCorrectly(int, int);
  void setCellProperties(int i, int j, QString alphagram, QStringList solutions, quint8 numSolutions);


 private:

  QTableWidgetItem* wordCells[FIXED_ROWS][FIXED_COLS];
  QStringList cellSolutions[FIXED_ROWS][FIXED_COLS];
  QString cellAlphagrams[FIXED_ROWS][FIXED_COLS];
  quint8 cellNumSolutions[FIXED_ROWS][FIXED_COLS];

};


#endif
