#include "mainwindow.h"
//#undef RAND_MAX
//#define RAND_MAX 0x1

quint32 lastGeneratedNum;
void csrand(quint32 seed)
{
	lastGeneratedNum = seed;
}

quint32 crand()
{
	lastGeneratedNum = (16807*lastGeneratedNum) % 2147483647;
	return lastGeneratedNum;
}

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
  currentlyAskingQuestion = false;
  
  questionTimer = new QTimer();
  connect(questionTimer, SIGNAL(timeout()), SLOT(timerTimedOut()));
  timeLimitSecs = 5;
  currentTime = 0;
	
	
	
	#ifdef Q_WS_MAC
	QSettings ZyzzyvaSettings("pietdepsi.com", "Zyzzyva");
	#else
	QSettings ZyzzyvaSettings("Piet Depsi", "Zyzzyva");
	#endif
	ZyzzyvaSettings.beginGroup("Zyzzyva");
	
	QString defaultUserDir = QDir::homePath() + "/.zyzzyva";
	QString zyzzyvaDataDir = QDir::cleanPath (ZyzzyvaSettings.value("user_data_dir", defaultUserDir).toString());
	
	if (QFile::exists(zyzzyvaDataDir + "/lexicons/OWL2+LWL.db"))
	{
		wordDb = QSqlDatabase::addDatabase("QSQLITE");
		wordDb.setDatabaseName(QDir::homePath() + "/.zyzzyva/lexicons/OWL2+LWL.db");
		wordDb.open();
	}
	else
	{
		QMessageBox::warning(this, "Zyzzyva not found", "A suitable Zyzzyva installation was not found. You will not "
		"be able to see definitions and hooks for the words at the end of each round. Zyzzyva is a free word study tool"
		", by Michael Thelen, found at <a href=""http://www.zyzzyva.net"">http://www.zyzzyva.net</a>.");
	}

  
}

void MainWindow::on_pushButtonLoad_clicked()
{
  // populate item list
	QDir dir = QDir::current();
	dir.cd("lists");
  QStringList list = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
  bool ok;
  QString wordList = QInputDialog::getItem(this, "Select word list", "Select word list", list, 0, false, &ok);
	if (!ok) return;
	
	// search if this word list is being worked on.
	QDir d;
	if (!d.exists("progress"))
	{
		d.mkdir("progress");
	
	}
	curWordList = wordList;
	
	//if (!QFile::exists("progress/" + wordList))
	//{
	// 	QFile file("progress/" + wordList);
// 			file.open(QIODevice::WriteOnly);
// 			QDataStream stream(&file);
// 			stream << (quint32)QDateTime::currentDateTime().toTime_t(); // random seed.
// 			stream << (quint32)0;	// seen questions
// 			stream << (quint32)0;	// current question number
// 			
// 			file.close();
	//}
	
	

	seenQuestions = 0;
	currentQuestionNumber = 0;
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
			QStringList q = line.simplified().split(" ");
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
	
	QFile file("progress/" + wordList);
	QDataStream stream;
	 stream.setVersion(QDataStream::Qt_4_4);
	if (!file.open(QIODevice::ReadOnly))
	{
		randomSeed = (quint32)QDateTime::currentDateTime().toTime_t();
	
	}
	else
	{
		stream.setDevice(&file);
		stream >> randomSeed >> seenQuestions >> currentQuestionNumber;
		
	}	
	ui.labelInfo->setText(QString::number(RAND_MAX));
	
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
		stream >> questions[i].correct;
		questions[i].asked = true;
	
	}
	
	file.close();
	
	currentlyAskingQuestion = false;
	displayQuestion();
	if (questions[currentQuestionNumber].asked == true)
		displayAnswers(questions[currentQuestionNumber].correct);
	else
		askQuestion();
	
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
	if (currentlyAskingQuestion) return;
	if (questions.size() == 0) return;
	
	if (currentQuestionNumber == 0) currentQuestionNumber = 0;
	else currentQuestionNumber--;
	displayQuestion();
	displayAnswers(questions.at(currentQuestionNumber).correct);
}

void MainWindow::on_pushButtonNext_clicked()
{
	if (currentlyAskingQuestion)
	{
	
		// answer correctly
		questions[currentQuestionNumber].correct = true;
		displayAnswers(true);
		currentlyAskingQuestion = false;
		questionTimer->stop();
		currentTime = 0;
	}
	else	// go on to next question.
	{
	
		if (questions.size() == 0) return;
		currentQuestionNumber++;
		if (currentQuestionNumber > questions.size() - 1) currentQuestionNumber = questions.size() - 1;
		displayQuestion();	// ask question
		if (questions.at(currentQuestionNumber).asked == true)
		{	// but if it's been asked just display answers
			displayAnswers(questions.at(currentQuestionNumber).correct);
			currentlyAskingQuestion = false;
		}
		else
		{	// really ask question
			askQuestion();
		}
	}
}

void MainWindow::askQuestion()
{
	currentlyAskingQuestion = true;
	questions[currentQuestionNumber].asked = true;
	seenQuestions = currentQuestionNumber + 1;
	/* start timer*/
	questionTimer->start(1000);
	ui.lcdNumberTimer->display(timeLimitSecs);
}

void MainWindow::displayQuestion()
{
	// currentQuestionNumber 0 is the first question
	// size - 1 is the last question
	ui.labelInfo->setText("Question " + QString::number(currentQuestionNumber + 1) + 
	" of " + QString::number(questions.size()) + " (" + 
	QString::number(questions.at(currentQuestionNumber).solutions.size()) + ")");
	
	ui.plainTextEdit->clear();
	
	QTextCursor cursor(ui.plainTextEdit->document());
	QTextCharFormat chFormat;

	QString spaces;
	for (int i = 0; i < 7; i++)
		spaces += " ";	
	
	chFormat.setFont(QFont("Courier New", 40, 66));
	cursor.insertText(spaces + questions.at(currentQuestionNumber).alphagram, chFormat);
	
	
	ui.plainTextEdit->setTextCursor(cursor);

}

void MainWindow::displayAnswers(bool correct)
{

	QTextCursor cursor = ui.plainTextEdit->textCursor();
	QTextCharFormat chFormat;
	if (correct)
		chFormat.setBackground(QBrush(QColor(150, 255, 150)));
	else
		chFormat.setBackground(QBrush(QColor(255, 150, 150)));
	cursor.insertBlock();
	chFormat.setFont(QFont("Arial", 14, 35));
	
	for (int i = 0; i < questions.at(currentQuestionNumber).solutions.size(); i++)
	{
		QString backHooks, frontHooks, definition, probability;
		if (wordDb.isOpen())
		{
			
			QSqlQuery query;
	
			query.exec("select front_hooks, back_hooks, definition, probability_order from words where word = '" 
			+ questions.at(currentQuestionNumber).solutions.at(i) + "'");
	
			while (query.next())
			{
				frontHooks = query.value(0).toString();
				backHooks = query.value(1).toString();
				definition = query.value(2).toString();
				probability = query.value(3).toString();
			}
		}
	
		cursor.insertText(frontHooks + "\t" + questions.at(currentQuestionNumber).solutions.at(i) 
		+ "\t" + backHooks + "\t" + definition, chFormat);
		cursor.insertBlock();
	}
	
	ui.plainTextEdit->setTextCursor(cursor);
}

void MainWindow::on_pushButtonMark_clicked()
{
	if (currentlyAskingQuestion)
	{
	
		// answer incorrectly
		questions[currentQuestionNumber].correct = false;
		displayAnswers(false);
		currentlyAskingQuestion = false;
		questionTimer->stop();
		currentTime = 0;
	}
	else
	{
		// we're going forwards or backwards
		questions[currentQuestionNumber].correct = !questions[currentQuestionNumber].correct;
		
		/* refresh question - answer */
		displayQuestion();
		displayAnswers(questions.at(currentQuestionNumber).correct);	
	
	}
}

void MainWindow::on_pushButtonWriteMissed_clicked()
{


	QFile file("progress/" + curWordList + "missed.txt");
	file.open(QIODevice::WriteOnly);
	QTextStream stream(&file);
	int numAlphagramsIncorrect = 0;
	int numWordsIncorrect = 0;
	for (quint32 i = 0; i < seenQuestions; i++)
	{
		if (!questions[i].correct)
		{
			numAlphagramsIncorrect++;
			for (int j = 0; j < questions[i].solutions.size(); j++)
			{
				numWordsIncorrect++;
				stream << questions[i].solutions[j] << endl;
			}
		}
	
	}
	file.close();
	ui.labelInfo->setText(QString("Wrote %1 words (%2 alphagrams out of %3)").arg(numWordsIncorrect).
								arg(numAlphagramsIncorrect).arg(seenQuestions));

}

void MainWindow::timerTimedOut()
{
	
	currentTime++;
	ui.lcdNumberTimer->display(timeLimitSecs - currentTime);
	if (currentTime >= timeLimitSecs)
	{
		questionTimer->stop();
		currentTime = 0;
		
		// mark missed -- answer incorrectly.
		
		questions[currentQuestionNumber].correct = false;
		displayAnswers(false);
		currentlyAskingQuestion = false;
	}
}
