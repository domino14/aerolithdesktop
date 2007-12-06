#include "wordsTableWidget.h"




wordsTableWidget::wordsTableWidget(int numRows, int numCols)
{
	this->numRows = numRows;
	this->numCols = numCols;
	setRowCount(numRows);
	setColumnCount(numCols);

	horizontalHeader()->hide();
	verticalHeader()->hide();

	setSelectionMode(QAbstractItemView::NoSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	setGridStyle(Qt::NoPen);

	for (int i = 0; i < numCols; i++)
		setColumnWidth(i, 150);

	for (int i = 0; i < numRows; i++)
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

	for (int i = 0; i < numRows; i++)
		for (int j = 0; j < numCols; j++)
		{
			wordsTableWidgetItem* item = new wordsTableWidgetItem;
			item->tableWidgetItem = new QTableWidgetItem("");
			item->tableWidgetItem->setTextAlignment(Qt::AlignCenter);
			item->tableWidgetItem->setFont(wordFont);
			setItem(i, j, item->tableWidgetItem);

			tableItems.append(item);
		}
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setFixedSize(752, 182); // argh
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

	for (int i = 0; i < numRows; i++)
		for (int j = 0; j < numCols; j++)
			tableItems[j+i*numCols]->tableWidgetItem->setFont(wordFont);

}

void wordsTableWidget::prepareForStart()
{
	QFont wordFont = tableItems[0]->tableWidgetItem->font();
	wordFont.setPointSize(6);
	changeFontSize(wordFont);

	for (int i = 0; i < numRows; i++)
		for (int j = 0; j < numCols; j++)
			tableItems[j+i*numCols]->tableWidgetItem->setFont(wordFont);
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
	for (int i = 0; i < numRows; i++)
		for (int j = 0; j < numCols; j++)
			item(i, j)->setText("");

}


QStringList wordsTableWidget::getCellSolutions(int i, int j)
{
	QStringList x;
	if (i > numRows - 1 || j > numCols - 1 || i < 0 || j < 0) return x;
	return tableItems[j+i*numCols]->cellSolutions;
}

QString wordsTableWidget::getCellAlphagram(int i, int j)
{
	QString x;
	if (i > numRows - 1 || j > numCols - 1 || i < 0 || j < 0) return x;
	return tableItems[j+i*numCols]->cellAlphagram;
}
quint8 wordsTableWidget::getCellNumSolutions(int i, int j)
{
	if (i > numRows - 1 || j > numCols - 1 || i < 0 || j < 0) return 0;
	return tableItems[j+i*numCols]->cellNumSolutions;
}

quint8 wordsTableWidget::getColorBrushIndex(quint8 numSolutions)
{
	Q_ASSERT(numSolutions > 0);
	if (numSolutions > 9) return 8;
	else return (numSolutions - 1);
}

void wordsTableWidget::answeredCorrectly(int row, int column)
{
	tableItems[column+row*numCols]->cellNumSolutions--;
	int numSolutions = tableItems[column+row*numCols]->cellNumSolutions;
	if (numSolutions > 0)
	{      
		item(row, column)->setBackground(colorBrushes[getColorBrushIndex(numSolutions)]);
	}
	else
		item(row, column)->setText("");
}

void wordsTableWidget::setCellProperties(int i, int j, QString alphagram, QStringList solutions, quint8 numSolutionsNotYetSolved)
{
	if (i > numRows - 1 || j > numCols - 1 || i < 0 || j < 0) return;
	tableItems[j+i*numCols]->cellAlphagram = alphagram;
	if (numSolutionsNotYetSolved > 0) 
	{
		wordLength = alphagram.length();
		item(i, j)->setText(alphagram);
		item(i, j)->setBackground(colorBrushes[getColorBrushIndex(numSolutionsNotYetSolved)]);
	}
	else item(i, j)->setText("");
	tableItems[j+i*numCols]->cellNumSolutions = numSolutionsNotYetSolved;
	tableItems[j+i*numCols]->cellSolutions = solutions;

}


void wordsTableWidget::alphagrammizeWords()
{
	// wordsWidget
	for (int i = 0; i < numRows; i++)
		for (int j = 0; j < numCols; j++)
			if (tableItems[j+i*numCols]->cellNumSolutions != 0) item(i,j)->setText(tableItems[j+i*numCols]->cellAlphagram);
}

void wordsTableWidget::shuffleWords()
{
	for (int i = 0; i < numRows; i++)
		for (int j = 0; j < numCols; j++)
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
