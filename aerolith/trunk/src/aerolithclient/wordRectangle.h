#ifndef _WORDRECTANGLE_H_
#define _WORDRECTANGLE_H_

#include <QtGui>
#include <QtCore>

class WordRectangle : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    WordRectangle();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);
    QRectF boundingRect() const;
    void setText(QString);
    void setFixedWidthFont(bool on);
    void showText();
    void hideText();
    void setTransparentBG();
    void setOpaqueBG();
    void alphagrammizeText();
    void shuffleText();
private:
    QString originalText;
    QString text;
    bool opaqueBG;
    bool shouldShowText;
    void mousePressEvent ( QGraphicsSceneMouseEvent * event);
    QString fontFamily;
signals:
    void mousePressed();
};

#endif
