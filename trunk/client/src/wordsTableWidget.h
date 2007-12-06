#ifndef _WORDS_TABLE_WIDGET_H_
#define _WORDS_TABLE_WIDGET_H_

#include <QtGui>

class wordsTableWidgetItem
{
public:
	QTableWidgetItem* tableWidgetItem;
	QStringList cellSolutions;
	QString cellAlphagram;
	quint8 cellNumSolutions;
};

class wordsTableWidget : public QTableWidget
{
	Q_OBJECT

public:
	int getNumRows()
	{
		return numRows;
	}
	int getNumCols()
	{
		return numCols;
	}
	wordsTableWidget(int, int);
	void clearCells();
	QStringList getCellSolutions(int, int);
	QString getCellAlphagram(int, int);
	quint8 getCellNumSolutions(int, int);
	QBrush colorBrushes[9]; // add to utilities file
	void answeredCorrectly(int, int);
	void setCellProperties(int i, int j, QString alphagram, QStringList solutions, quint8 numSolutions);
	quint8 getColorBrushIndex(quint8 numSolutions);
	void changeFontSize(QFont&);
	quint8 wordLength;
	void prepareForStart();
private:

	bool fixedWidth;
	QString shuffleString(QString);

	QList <wordsTableWidgetItem*> tableItems;

	int numRows, numCols;

	public slots:
		void changeFont();
		void alphagrammizeWords();
		void shuffleWords();
		void wordsWidgetItemClicked(QTableWidgetItem* clickedItem);
};



#endif
