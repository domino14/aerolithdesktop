#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QWidget(parent)
{
  ui.setupUi(this);
  
  QShortcut* shortcutLoad = new QShortcut(QKeySequence(Qt::Key_L), this);
  QShortcut* shortcutSave = new QShortcut(QKeySequence(Qt::Key_S), this);
  QShortcut* shortcutMark = new QShortcut(QKeySequence(Qt::Key_M), this);
  QShortcut* shortcutPrevious = new QShortcut(QKeySequence(Qt::Key_P), this);
  QShortcut* shortcutNext = new QShortcut(QKeySequence(Qt::Key_N), this);

  QShortcut* shortcutSpace = new QShortcut(QKeySequence(Qt::Key_Space), this);
 
  connect(shortcutLoad, SIGNAL(activated()), ui.pushButtonLoad, SLOT(animateClick()));
  connect(shortcutSave, SIGNAL(activated()), ui.pushButtonSave, SLOT(animateClick()));
  connect(shortcutMark, SIGNAL(activated()), ui.pushButtonMark, SLOT(animateClick()));
  connect(shortcutPrevious, SIGNAL(activated()), ui.pushButtonPrevious, SLOT(animateClick()));
  connect(shortcutNext, SIGNAL(activated()), ui.pushButtonNext, SLOT(animateClick()));
  connect(shortcutSpace, SIGNAL(activated()), ui.pushButtonNext, SLOT(animateClick()));

  
  this->show();
  currentQuestionNumber = 0;
  seenQuestions = 0;
  
  ui.plainTextEdit->document()->setMaximumBlockCount(1000);
}

void MainWindow::on_pushButtonLoad_clicked()
{
  // populate item list
	QDir dir = QDir::current();
	dir.cd("lists");
  QStringList list = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
  bool ok;
  QString wordList = QInputDialog::getItem(this, "Select word list", "Select word list", list, 0, false, &ok);
	if (ok)
	{
		// search if this word list is being worked on.
		QDir d;
		if (!d.exists("progress"))
		{
			d.mkdir("progress");
		
		}
		curWordList = wordList;
		
		if (!QFile::exists("progress/" + wordList))
		{
			QFile file("progress/" + wordList);
			file.open(QIODevice::WriteOnly);
			QDataStream stream(&file);
			stream << (quint32)QDateTime::currentDateTime().toTime_t(); // random seed.
			stream << (quint32)0;	// seen questions
			stream << (quint32)0;	// current question number
			
			file.close();
		}
		
		QFile file("progress/" + wordList);
		file.open(QIODevice::ReadOnly);
		QDataStream stream(&file);
		
		stream >> randomSeed >> seenQuestions >> currentQuestionNumber;
		
		ui.labelInfo->setText("Please wait... loading data");
		/* populate data structure */
		QFile wordListFile("lists/" + wordList);
		wordListFile.open(QIODevice::ReadOnly | QIODevice::Text);
		QTextStream textStream(&wordListFile);
		
		questions.clear();
		
		while (!textStream.atEnd())
		{
			QString line = textStream.readLine();
			if (line != "")
			{
				QStringList q = line.split(" ");
				Question tmp;
				tmp.alphagram = q.at(0);
				tmp.solutions = q.mid(1);
				tmp.correct = false;
				questions << tmp;
			}
		
		}
		wordListFile.close();
		ui.labelInfo->setText("Loaded data.");
		
		/* randomize the list according to the random seed */
		
		qsrand(randomSeed);
		for (int i = 0; i < questions.size(); i++)
		{
			Question temp;
			int randomIndex = qrand() % questions.size();
			temp = questions[i];
			questions[i] = questions[randomIndex];
			questions[randomIndex] = temp;
		}
		
		
		/* set the appropriate questions true or false for "correct"*/
		for (quint32 i = 0; i < seenQuestions; i++)
		{
			bool correct;
			stream >> correct;
			questions[i].correct = correct;
		
		}
		
		file.close();


	}
	displayQuestion();
}



void MainWindow::on_pushButtonSave_clicked()
{
	QFile file("progress/" + curWordList);
	file.open(QIODevice::WriteOnly);
	QDataStream stream(&file);
	stream << randomSeed; 
	stream << seenQuestions;
	stream << currentQuestionNumber;
	
	for (quint32 i = 0; i < seenQuestions; i++)
	{
		stream << questions.at(i).correct;
	}
	
			
	file.close();
	ui.labelInfo->setText("Saved data.");
}

void MainWindow::on_pushButtonPrevious_clicked()
{
	if (questions.size() == 0) return;
	if (currentQuestionNumber == 0) currentQuestionNumber = 0;
	else currentQuestionNumber--;
	displayQuestion();
}

void MainWindow::on_pushButtonNext_clicked()
{
	if (questions.size() == 0) return;
	currentQuestionNumber++;
	if (currentQuestionNumber > questions.size() - 1) currentQuestionNumber = questions.size() - 1;
	displayQuestion();
}

void MainWindow::displayQuestion()
{
	// currentQuestionNumber 0 is the first question
	// size - 1 is the last question
	ui.labelInfo->setText("Question " + QString::number(currentQuestionNumber + 1) + 
	" of " + QString::number(questions.size()));
	
	ui.plainTextEdit->clear();
	
	QTextCursor cursor(ui.plainTextEdit->document());
	QTextCharFormat chFormat;

	QString spaces;
	for (int i = 0; i < 10; i++)
		spaces += " ";	
	
	chFormat.setFont(QFont("Courier New", 24, 66));
	cursor.insertText(spaces + questions.at(currentQuestionNumber).alphagram, chFormat);
	
	cursor.insertBlock();
	chFormat.setFont(QFont("Arial", 14, 35));
	for (int i = 0; i < questions.at(currentQuestionNumber).solutions.size(); i++)
	{
		cursor.insertText(questions.at(currentQuestionNumber).solutions.at(i), chFormat);
		cursor.insertBlock();
	}
	
	
	ui.plainTextEdit->setTextCursor(cursor);

	
}

void MainWindow::on_pushButtonMark_clicked()
{
}
