#include "wordsTableWidget.h"




wordsTableWidget::wordsTableWidget()
{
  setRowCount(FIXED_ROWS);
  setColumnCount(FIXED_COLS);
  
  horizontalHeader()->hide();
  verticalHeader()->hide();

  setSelectionMode(QAbstractItemView::NoSelection);
  setEditTriggers(QAbstractItemView::NoEditTriggers);

  for (int i = 0; i < FIXED_COLS; i++)
    setColumnWidth(i, 150);

  for (int i = 0; i < FIXED_ROWS; i++)
    setRowHeight(i, 20);

  /*
#ifdef Q_OS_MAC
  QFont wordFont("Helvetica", 16, QFont::Black);
#else
  QFont wordFont("Helvetica", 12, QFont::Black);
#endif
  */
  QFont wordFont("Helvetica", 6, QFont::Black);


  colorBrushes[0].setColor(Qt::black);
  colorBrushes[1].setColor(Qt::darkGreen);
  colorBrushes[2].setColor(Qt::blue);
  colorBrushes[3].setColor(Qt::darkCyan);
  colorBrushes[4].setColor(Qt::green);
  colorBrushes[5].setColor(Qt::darkMagenta);
  colorBrushes[6].setColor(Qt::darkRed);
  colorBrushes[7].setColor(Qt::red);
  colorBrushes[8].setColor(Qt::magenta);
  
  for (int i = 0; i < FIXED_ROWS; i++)
    for (int j = 0; j < FIXED_COLS; j++)
      {
	wordCells[i][j] = new QTableWidgetItem("");
	wordCells[i][j]->setTextAlignment(Qt::AlignCenter);
	wordCells[i][j]->setFont(wordFont);
	setItem(i, j, wordCells[i][j]);
      }
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setFixedSize(755, 185); // argh  -- 752 182
  setFocusPolicy(Qt::NoFocus);

  connect(this, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(wordsWidgetItemClicked(QTableWidgetItem*)));
  fixedWidth = false;
}

void wordsTableWidget::wordsWidgetItemClicked(QTableWidgetItem* clickedItem)
{
  int i = clickedItem->row();
  int j = clickedItem->column();
  item(i, j)->setText(shuffleString(item(i, j)->text()));
}

void wordsTableWidget::changeFont()
{
  QFont wordFont;

  wordFont.setPointSize(6); // start here
  if (fixedWidth == false)
    {
      wordFont.setFamily("Courier New");
      wordFont.setStyleHint(QFont::TypeWriter);
      wordFont.setWeight(QFont::Black);
      
      fixedWidth = true;
    }
  else
    {
      wordFont.setFamily("Helvetica");
      wordFont.setStyleHint(QFont::SansSerif);
      wordFont.setWeight(QFont::Black);
 
      fixedWidth = false;
    }

  changeFontSize(wordFont);

  for (int i = 0; i < FIXED_ROWS; i++)
    for (int j = 0; j < FIXED_COLS; j++)
      wordCells[i][j]->setFont(wordFont);
  
}

void wordsTableWidget::prepareForStart()
{
	QFont wordFont = wordCells[0][0]->font();
	wordFont.setPointSize(6);
	changeFontSize(wordFont);

  for (int i = 0; i < FIXED_ROWS; i++)
    for (int j = 0; j < FIXED_COLS; j++)
      wordCells[i][j]->setFont(wordFont);
}


void wordsTableWidget::changeFontSize(QFont& wordFont)
{

  QString testStr(wordLength, 'M');
  
  bool stillFits = true;
  while (stillFits)
    {
      wordFont.setPointSize(wordFont.pointSize() + 1);
      QFontMetrics fm(wordFont);
      if (fm.boundingRect(testStr).width() < 135 && fm.boundingRect(testStr).height() < 20)
	stillFits = true;
      else
	stillFits = false;
	  qDebug() << "Test rectangle for" << testStr << "point size" << wordFont.pointSize() 
			<< "width" << fm.boundingRect(testStr).width() << "height" << fm.boundingRect(testStr).height();
    }
  
	qDebug() << "Set point size to " << wordFont.pointSize();
  
}


void wordsTableWidget::clearCells()
{
  for (int i = 0; i < FIXED_ROWS; i++)
    for (int j = 0; j < FIXED_COLS; j++)
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

quint8 wordsTableWidget::getColorBrushIndex(quint8 numSolutions)
{
  Q_ASSERT(numSolutions > 0);
  if (numSolutions > 9) return 8;
  else return (numSolutions - 1);
}

void wordsTableWidget::answeredCorrectly(int row, int column)
{
  Q_ASSERT(cellNumSolutions[row][column] > 0);
  cellNumSolutions[row][column]--;
  int numSolutions = cellNumSolutions[row][column];
  if (numSolutions > 0)
    {      
      item(row, column)->setForeground(colorBrushes[getColorBrushIndex(numSolutions)]);
    }
  else
    item(row, column)->setText("");
}

void wordsTableWidget::setCellProperties(int i, int j, QString alphagram, QStringList solutions, quint8 numSolutionsNotYetSolved)
{
  cellAlphagrams[i][j] = alphagram;
  if (numSolutionsNotYetSolved > 0) 
    {
      wordLength = alphagram.length();
      item(i, j)->setText(alphagram);
      item(i, j)->setForeground(colorBrushes[getColorBrushIndex(numSolutionsNotYetSolved)]);
    }
  else item(i, j)->setText("");
  cellNumSolutions[i][j] = numSolutionsNotYetSolved;
  cellSolutions[i][j] = solutions;

}


void wordsTableWidget::alphagrammizeWords()
{
	// wordsWidget
  for (int i = 0; i < FIXED_ROWS; i++)
    for (int j = 0; j < FIXED_COLS; j++)
      if (cellNumSolutions[i][j] != 0) item(i,j)->setText(cellAlphagrams[i][j]);
}

void wordsTableWidget::shuffleWords()
{
  for (int i = 0; i < FIXED_ROWS; i++)
    for (int j = 0; j < FIXED_COLS; j++)
      item(i,j)->setText(shuffleString(item(i,j)->text()));
}

QString wordsTableWidget::shuffleString(QString inputString)
{
  for (int i = 0; i < inputString.length(); i++)
    {
      int j = qrand() % inputString.length();
      QChar tmp;
      tmp = inputString[i];
      inputString[i] = inputString[j];
      inputString[j] = tmp;
      
    }
  return inputString;
}
