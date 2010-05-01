#include "dawg.h"

Dawg::Dawg()
{
}

void Dawg::readDawg(QString filename)
{
    /* reads a dawg that was created with Mark Wutka's GPLed "makedawg.c" file.
        "makedawg.c" was modified by me (Cesar) in a few ways:
        - letter fits in 5 bits rather than 8, so 0-25 is A-Z, 26 27 28 are special and used for spanish
        - the first 4 bytes of the dawg show the offset from the end. basically dawg size (in nodes) - 1 - offset is
            where the dawg begins. offset would always be 26 in the english dictionary but it's 27 in the spanish one,
            so i needed to add something to differentiate it.


       -----------
       to make the spanish dawg, the fise list must be modified - replace the n-tilde with 'k' (which is unused in spanish)
       then sort the list and run makedawg.c on it. i'm too lazy to figure out why makedawg.c won't work with n-tilde.
    */
    nodes.clear();
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return;
    QDataStream in(&file);
    QVector <quint32> ints;
    while (!in.atEnd())
    {
        quint32 integer;
        in >> integer;
        ints.append(integer);
    }
    file.close();
    qDebug() << ints.size() << "nodes read.";
    bool endOfNext;
    unsigned char letter;
    bool endOfWord;
    int child, sibling;
    int offsetFromEnd = (ints[0] & 255);
    qDebug() << "Read offsetfromend" << offsetFromEnd;
    if (offsetFromEnd <= 26) offsetFromEnd = 26;
    nodes.append(DawgNode(NULL_LETTER, true, ints.size() - offsetFromEnd, NULL_NODE));
    //00 00 00 60
    for (int i = 1; i < ints.size(); i++)
    {

        unsigned char byte0 = (ints[i]) & 255;
        letter = byte0 & 31;
        if (letter <= 25) letter += 'A';
        else if (letter >= 26 && letter <= 29) letter += '1' - 26;
        else letter = NULL_LETTER;
        if ( (byte0 >> 5) & 1 == 1) endOfWord = true;
        else endOfWord = false;
        if ( (byte0 >> 6) & 1 == 1) endOfNext = true;
        else endOfNext = false;
        child = ints[i] >> 7;
        if (child == 0) child = NULL_NODE;
        if (endOfNext)
            sibling = NULL_NODE;
        else
            sibling = i + 1;

        nodes.append(DawgNode(letter, endOfWord, child, sibling));

    }

}

bool Dawg::findWord(QByteArray wordToFind)
{
    int a;
    int parent;
    int wordlength;
    bool letterfound;
    int nextsibling;
    int child;
    parent = 0;
    wordlength = wordToFind.length();
    char thisletter;
    for (a = 0; a < wordlength; a++)
    {
        thisletter = toupper(wordToFind[a]);
        letterfound = false;

        child = nodes.at(parent).child;
        if (child == NULL_NODE) return 0;
        if (nodes.at(child).letter == thisletter)
        {
            letterfound = true;
            parent = child;
        }
        else
        {       //traverse through siblings
            nextsibling = nodes.at(child).sibling;
            while (nextsibling != NULL_NODE)
            {
                if (nodes.at(nextsibling).letter == thisletter)
                {
                    letterfound = true;
                    parent = nextsibling;
                }
                nextsibling = nodes.at(nextsibling).sibling;

            }
            if (!letterfound) return false;
        }


    }
    return nodes.at(parent).endOfWord;

}

int Dawg::findPartialWord(QByteArray wordToFind)
{
    int a;
    int parent;
    bool letterfound;
    int nextsibling;
    int foundnode;
    int child;
    parent = 0;

    for (a = 0; a < wordToFind.length(); a++)
    {
        letterfound = false;
        child = nodes.at(parent).child;
        if (child == NULL_NODE) return 0;
        if (nodes.at(child).letter == toupper(wordToFind[a]))
        {
            letterfound = true;
            parent = child;
            foundnode = child;
        }
        else
        {       //traverse through siblings
            nextsibling = nodes.at(child).sibling;
            while (nextsibling != NULL_NODE)
            {
                if (nodes.at(nextsibling).letter == toupper(wordToFind[a]))
                {
                    letterfound = true;
                    parent = nextsibling;
                    foundnode = nextsibling;
                }
                nextsibling = nodes.at(nextsibling).sibling;
            }
            if (!letterfound) return NULL_NODE;
        }


    }
    return foundnode;

}


void Dawg::printDawg()
{
    qDebug() << "\tletter\tchild\tsibling\teow";
    for (int i = 0; i < nodes.size(); i++)
        qDebug() << i << ":\t"<< nodes[i].letter << "\t" <<
                nodes[i].child << "\t" << nodes[i].sibling << "\t" <<
                nodes[i].endOfWord;

}

QByteArray Dawg::findHooks(QByteArray word)
{
    QByteArray hooks;
    int node = findPartialWord(word);

    if (node != NULL_NODE)
    {
        /* traverse thru children*/
        int child = nodes.at(node).child;
        if (child == NULL_NODE) return hooks;
        if (nodes.at(child).endOfWord)
        {
            hooks += nodes.at(child).letter;
        }
        //traverse through siblings
        int nextsibling = nodes.at(child).sibling;
        while (nextsibling != NULL_NODE)
        {
            if (nodes.at(nextsibling).endOfWord)
            {
                hooks += nodes.at(nextsibling).letter;
            }
            nextsibling = nodes.at(nextsibling).sibling;
        }

    }
    return hooks;

}

void Dawg::checkDawg(QString wordList)
{

    QFile file(wordList);
    if (!file.open(QIODevice::ReadOnly)) return;
    while (!file.atEnd())
    {
        QByteArray line = file.readLine();
        line = line.simplified();
        QList <QByteArray> words = line.split(' ');
        if (line.length() > 1 && !findWord(words[0]))
            qDebug() << line << "not found!";
    }
    file.close();
    qDebug() << "Checked every word in" << wordList;
}
