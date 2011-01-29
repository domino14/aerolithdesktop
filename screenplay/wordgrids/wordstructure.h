#ifndef WORDSTRUCTURE_H
#define WORDSTRUCTURE_H

#include <QObject>
#include <QtSql>


class WordStructure : public QThread
{
    Q_OBJECT
public:
    WordStructure(QObject* parent);
    enum ThreadOptions
    {
        GENERATE_WORD_HASH, RECURSIVE_GRID_CHECK
            };
    ThreadOptions currentThreadOption;
    QHash <QString, QString> wordStructure;
    void loadWordStructure(QString dbName);
    QString getDefinitions(QString words);
    QSqlDatabase wordDb;
    bool containsOnlyPlurals(QString words);
protected:
    void run();
    void generateWordHash();

signals:
    void finishedLoadingWordStructure();
};

#endif // WORDSTRUCTURE_H
