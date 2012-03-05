/*
** Mame HQ -- yet another front end for mame!
**
** by jens kaas benner, October 2010
**    jens@bennerhq.com
*/

/*
** "THE BEER-WARE LICENSE" (Revision 42):
** <jens@bennerhq.com> wrote this file. As long as you retain this notice you
** can do whatever you want with this stuff. If we meet some day, and you think
** this stuff is worth it, you can buy me a beer in return, /benner.
*/

#ifndef WIDGET_H
#define WIDGET_H

#include "getroms.h"

#include <QDebug>
#include <QtGui/QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimeLine>
#include <QGraphicsWidget>
#include <QLabel>

// ----------------------------------------------------------------------------

class LetterWidget : public QWidget
{
    Q_OBJECT

private:
    QTimeLine *timer;
    QString m_txt;
    qreal fade;

public slots:
    void nxt(int frame);
    void fin();

public:
    LetterWidget(QWidget *parent);
    void paintEvent(QPaintEvent *parent);

    void setMe(int ix, int iy, QString s);
    void setLetter(QString m_txt);
    bool haveLetter() { return m_txt != ""; }
};

// ----------------------------------------------------------------------------

class SnapWidget : public QWidget
{
private:
    QPixmap *m_snap;
    QPixmap *m_cabinet;

public:
    SnapWidget(QWidget *parent) : QWidget(parent), m_snap(NULL), m_cabinet(NULL) {}
    ~SnapWidget();

    void paintEvent(QPaintEvent *parent);

    void setSnap(int idx);
};

// ----------------------------------------------------------------------------

class Widget : public QWidget
{
    Q_OBJECT

    QGraphicsScene *scene;
    QGraphicsView *view;

    QTimeLine *timer;
    LetterWidget *letter;
    SnapWidget *snap;

    int romsIdx;
    int selected;
    qreal more;

    bool keyPressed;

private:
    void setView();
    int getSelected();
    int getViewY();

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    void resizeEvent (QResizeEvent *event);

public slots:
    void nextAnimate(int i);
    void finishedAnimate();

public:
    Widget(QWidget *parent = 0);
    ~Widget();

    void paintEvent(QPaintEvent *parent);
};

#endif // WIDGET_H
