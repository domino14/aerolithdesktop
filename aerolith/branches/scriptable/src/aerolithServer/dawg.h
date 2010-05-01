#ifndef DAWG_H
#define DAWG_H

#include <QtCore>

#define NULL_LETTER 0
#define NULL_NODE -1

struct DawgNode
{
    unsigned char letter;
    int child;
    int sibling;
    bool endOfWord;
    DawgNode(unsigned char let, bool eow, int c, int s)
    {
        letter = let; endOfWord = eow; child = c; sibling = s;
    }
    DawgNode()
    {
    }
};


class Dawg
{
public:
    Dawg();
    void readDawg(QString filename);
    void printDawg();
    bool findWord(QByteArray wordToFind);
    void checkDawg(QString wordList);
    int findPartialWord(QByteArray wordToFind);
    QByteArray findHooks(QByteArray word);
private:
    QVector <DawgNode> nodes;
};

#endif // DAWG_H
