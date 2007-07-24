#include "wordsTableWidget.h"


wordsTableWidget::wordsTableWidget()
{
  setRowCount(FIXED_ROWS);
  setColumnCount(FIXED_COLS);
  
  horizontalHeader()->hide();
  verticalHeader()->hide();

  setSelectionMode(QAbstractItemView::NoSelection);
  setEditTriggers(QAbstractItemView::NoEditTriggers);

  for (int i = 0; i < 5; i++)
    setColumnWidth(i, 150);

  for (int i = 0; i < 9; i++)
    setRowHeight(i, 20);

#ifdef Q_OS_MAC
  QFont wordFont("Arial Black", 16, QFont::Normal);
#else
  QFont wordFont("Arial Black", 12, QFont::Normal);
#endif

  colorBrushes[0].setColor(Qt::black);
  colorBrushes[1].setColor(Qt::darkGreen);
  colorBrushes[2].setColor(Qt::blue);
  colorBrushes[3].setColor(Qt::darkCyan);
  colorBrushes[4].setColor(Qt::green);
  colorBrushes[5].setColor(Qt::darkMagenta);
  colorBrushes[6].setColor(Qt::darkRed);
  colorBrushes[7].setColor(Qt::red);
  colorBrushes[8].setColor(Qt::magenta);
  
  for (int i = 0; i < 9; i++)
    for (int j = 0; j < 5; j++)
      {
	wordCells[i][j] = new QTableWidgetItem("");
	wordCells[i][j]->setTextAlignment(Qt::AlignCenter);
	wordCells[i][j]->setFont(wordFont);
	setItem(i, j, wordCells[i][j]);
      }
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setFixedSize(752, 182); // argh
  setFocusPolicy(Qt::NoFocus);

}

void wordsTableWidget::clearCells()
{
  for (int i = 0; i < 9; i++)
    for (int j = 0; j < 5; j++)
      item(i, j)->setText("");

}


QStringList wordsTableWidget::getCellSolutions(int i, int j)
{
  return cellSolutions[i][j];
}

QString wordsTableWidget::getCellAlphagram(int i, int j)
{
  return cellAlphagrams[i][j];
}
quint8 wordsTableWidget::getCellNumSolutions(int i, int j)
{

  return cellNumSolutions[i][j];
}

void wordsTableWidget::answeredCorrectly(int row, int column)
{
  cellNumSolutions[row][column]--;
  if (cellNumSolutions[row][column] > 0)
    item(row, column)->setForeground(wordsTableWidget::colorBrushes[(cellNumSolutions[row][column] > 9 ? 8 : (cellNumSolutions[row][column] - 1))]);
  else
    item(row, column)->setText("");


}


void wordsTableWidget::setCellProperties(int i, int j, QString alphagram, QStringList solutions, quint8 numSolutionsNotYetSolved)
{
  cellAlphagrams[i][j] = alphagram;
  if (numSolutionsNotYetSolved > 0) item(i, j)->setText(alphagram);
  else item(i, j)->setText("");
  item(i, j)->setForeground(colorBrushes[(numSolutionsNotYetSolved > 9 ? 8 : (numSolutionsNotYetSolved - 1))]);
  cellNumSolutions[i][j] = numSolutionsNotYetSolved;
  cellSolutions[i][j] = solutions;

}
