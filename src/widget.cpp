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

#include "widget.h"
#include "getroms.h"

#include <QDebug>
#include <QPainter>
#include <QGraphicsTextItem>
#include <QStyleOptionGraphicsItem>
#include <QKeyEvent>
#include <QApplication>
#include <QGraphicsWidget>
#include <QScrollBar>
#include <QWheelEvent>
#include <QGraphicsSceneWheelEvent>
#include <QApplication>
#include <QCursor>
#include <QPixmap>
#include <QFontMetrics>
#include <QProcess>

// ----------------------------------------------------------------------------

#define PIX         (12)
#define STEPS       (9)
#define HEIGHT      (PIX * STEPS)
#define TIMER       (220)

// ----------------------------------------------------------------------------

#define LETTER_DESC (0)
#define LETTER_YEAR (1)

LetterWidget::LetterWidget(QWidget *parent)
    : QWidget(parent), m_txt("")
{
    timer = new QTimeLine(500); /* 1/2 sec */
    timer->setCurveShape(QTimeLine::LinearCurve);
    connect(timer, SIGNAL(frameChanged(int)), this, SLOT(nxt(int)));
    connect(timer, SIGNAL(finished()), this, SLOT(fin()));
}

void LetterWidget::setMe(int ix, int iy, QString s)
{
    QFont f("Verdana");
    f.setPixelSize(120);
    f.setBold(true);

    QFontMetrics m(f);
    int iw = m.width(s) * 1.1;

    setGeometry(ix - iw - 40, iy + 40, iw, m.xHeight() * 2);
}

void LetterWidget::setLetter(QString txt)
{
    fade = 0.0;

    if (txt != "") {
        m_txt = txt;
        if (!isVisible()) {
            timer->stop();
            show();
        }
    }
    else if (timer->state() != QTimeLine::Running) {
        timer->setFrameRange(0, 100);
        timer->start();
    }
}

void LetterWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QFont font("Verdana");
    font.setPixelSize(120);
    font.setBold(true);

    // Draw rounded corner semi transparent background
    painter.save();
    painter.setBrush(Qt::black);
    painter.setOpacity(qMax(0.4 - fade, 0.0));
    painter.drawRoundedRect(rect(), 25, 25);
    painter.restore();

    // Draw thin solid white frame
    QRectF r = QRectF(rect().x(), rect().y(), rect().width() - 1, rect().height() - 1);
    painter.setOpacity(qMax(1.0 - fade, 0.0));
    painter.setPen(QPen(Qt::white, 1));
    painter.drawRoundedRect(r, 25, 25);

    // Draw *big* solid letter
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.setFont(font);
    painter.drawText(rect(), Qt::AlignCenter, m_txt);
}

void LetterWidget::nxt(int frame)
{
    fade = (qreal) frame / (qreal) timer->endFrame();
    update();
}

void LetterWidget::fin()
{
    hide();
    m_txt = "";
}

// ----------------------------------------------------------------------------

SnapWidget::~SnapWidget()
{
    if (m_snap) delete m_snap;
    if (m_cabinet) delete m_cabinet;
}

void SnapWidget::setSnap(int idx)
{
    if (m_snap) {
        delete m_snap;
        m_snap = NULL;
    }
    if (m_cabinet) {
        delete m_cabinet;
        m_cabinet = NULL;
    }
    if (idx >= 0 && idx < roms.size()) {
        QPixmap work1(path_cabinets + roms.at(idx).name + ".png");
        if (!work1.isNull()) {
            int w = qMin(work1.width(), rect().width());
            int h = qMin(work1.height(), rect().height()/2);
            m_cabinet = new QPixmap(work1.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }

        QPixmap work2(path_snaps + roms.at(idx).name + ".png");
        if (!work2.isNull()) {
            int w = qMin(work2.width() * 4, rect().width());
            int h = qMin(work2.height() * 4, rect().height()/2);
            m_snap = new QPixmap(work2.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));

            QPainter painter(m_snap);
            QRect rect(0, 0, m_snap->width(), m_snap->height());
            painter.setPen(QPen(Qt::black, 15));
            painter.drawRoundedRect(rect, 19.0, 19.0);

            painter.setPen(QPen(Qt::white, 1));
            rect = QRect(7, 7, m_snap->width() - 13, m_snap->height() - 13);
            painter.drawRoundedRect(rect, 19.0, 19.0);

            if (!m_cabinet) {
                m_cabinet = m_snap;
                m_snap = NULL;
            }
        }
    }

    if (m_cabinet || m_snap) {
        show();
    }
    else {
        hide();
    }
}

void SnapWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (m_cabinet) {
        int p = 0;
        if (!m_snap) p = rect().height() / 2 - m_cabinet->height() / 2;
        painter.drawPixmap(rect().width() / 2 - m_cabinet->width() / 2, p, *m_cabinet);

        if (m_snap) {
            painter.drawPixmap(rect().width() / 2 - m_snap->width() / 2,
                               m_cabinet->height() + 4 + rect().height() / 4 - m_snap->height() / 2,
                               *m_snap);
        }
    }
}

// ----------------------------------------------------------------------------

int globalW;        // need the with from the QGraphicsView ...
                    // this is the easy way to do it!!!??

class RomItem : public QGraphicsWidget
{
private:
    int m_romno;
    QPixmap *m_snap;

    void loadSnap();

public:
    RomItem(int romno);
    ~RomItem();

    QRectF boundingRect() const;
    QRectF drawRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    int getRom() { return m_romno; }
};

RomItem::RomItem(int romno)
    : m_romno(romno), m_snap(NULL)
{
//    loadSnap();
}

RomItem::~RomItem()
{
    if (m_snap) delete m_snap;
}

QRectF RomItem::boundingRect() const
{
    return QRectF(0, 0, globalW, HEIGHT);
}

QRectF RomItem::drawRect() const
{
    return QRectF(boundingRect().x() + 8,
                  boundingRect().y() + 4,
                  boundingRect().width() - 14,
                  boundingRect().height() - 8);
}

void RomItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QRectF bound = drawRect();

    if (!m_snap) {
        loadSnap();
    }

    QRectF rect;
    if (m_snap) {
        painter->drawPixmap(
                bound.x() + (bound.width() - m_snap->width()),
                bound.y() + (bound.height() - m_snap->height()) / 2,
                *m_snap);
        rect = QRectF(bound.x(), bound.y(),
                      bound.width() - m_snap->width(), bound.height());
    }
    else {
        rect = bound;
        rect.setWidth(rect.width() * 0.7);
    }
    QString txt1 = roms.at(m_romno).description;
    QString txt2 = roms.at(m_romno).manufacturer + " / " + roms.at(m_romno).year;

    int j = txt1.lastIndexOf("(");
    if (j > 0) {
        QString work = txt1.mid(j,txt1.length());
        txt2 = work.mid(1, work.length() - 2) + "\n" + txt2;
        txt1.remove(j, txt1.length());
    }

    QRectF org = rect;
    QFont fontName("Gill Sans MT");
    fontName.setPixelSize(28);
    fontName.setBold(true);
    painter->setFont(fontName);
    painter->setPen(Qt::white);
    painter->drawText(rect, Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, txt1, &rect);

    rect = QRectF(rect.x(), rect.y() + rect.height(),
                  org.width(), bound.height() - rect.x() - rect.height());

    QFont fontInfo("Verdana");
    fontInfo.setPixelSize(18);
    painter->setFont(fontInfo);
    painter->setPen(Qt::darkGray);
    painter->drawText(rect,Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, txt2, &rect);
}

void RomItem::loadSnap()
{
    QRectF bound = drawRect();

    if (m_snap) {
        delete m_snap;
        m_snap = NULL;
    }

    QPixmap work(path_marquees + roms.at(m_romno).name + ".png");
    if (!work.isNull()) {
        int w = qMin(work.width(), (int) bound.width());
        int h = qMin(work.height(), (int) bound.height());

        m_snap = new QPixmap(work.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        if (m_snap) {
            QPainter painter(m_snap);
            QRect rect(0, 0, m_snap->width(), m_snap->height());
            painter.setPen(QPen(Qt::black, 15));
            painter.drawRoundedRect(rect, 19.0, 19.0);

            painter.setPen(QPen(Qt::white, 1));
            rect = QRect(7, 7, m_snap->width() - 13, m_snap->height() - 13);
            painter.drawRoundedRect(rect, 19.0, 19.0);
        }
    }
}

// ----------------------------------------------------------------------------

Widget::Widget(QWidget *parent)
    : QWidget(parent), scene(NULL), view(NULL), romsIdx(-1),
      selected(-1), more(-1), keyPressed(false)
{
    // ESC will leave this app, always!!!
    grabKeyboard();

    // Letter indicator window
    letter = new LetterWidget(this);

    // Game snapshot window
    snap = new SnapWidget(this);

    // setup the animation timer
    timer = new QTimeLine(TIMER);
    timer->setCurveShape(QTimeLine::LinearCurve);
    connect(timer, SIGNAL(frameChanged(int)), this, SLOT(nextAnimate(int)));
    connect(timer, SIGNAL(finished()), this, SLOT(finishedAnimate()));

    // go to full screen. this will generate a resize event
    // there we will do the rest of the setup stuff
    setWindowState(windowState() | Qt::WindowFullScreen);
}

Widget::~Widget()
{
    releaseKeyboard();
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    }
    else if (event->key() == Qt::Key_Up) {
        if (timer->state() != QTimeLine::Running) {
            timer->setFrameRange(selected - HEIGHT, selected);
            timer->setDirection(QTimeLine::Backward);
            timer->start();

            keyPressed = true;
            more = 1;
        }
    }
    else if (event->key() == Qt::Key_Down) {
        if (timer->state() != QTimeLine::Running) {
            timer->setFrameRange(selected, selected + HEIGHT);
            timer->setDirection(QTimeLine::Forward);
            timer->start();

            keyPressed = true;
            more = 1;
        }
    }
    else if (event->key() == Qt::Key_Left) {
        romsIdx --;
        if (romsIdx < 1) romsIdx = romsView.size() - 1;
        setView();
    }
    else if (event->key() == Qt::Key_Right) {
        romsIdx ++;
        if (romsIdx >= romsView.size()) romsIdx = 1;
        setView();
    }
    else {
        event->ignore();
    }

    if (timer->state() == QTimeLine::Running) {
        snap->setSnap(-1);
    }
}

void Widget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
    }
    else {
        more = 0;

        if (event->key() == Qt::Key_Up) {
            keyPressed = false;
        }
        else if (event->key() == Qt::Key_Down) {
            keyPressed = false;
        }
        else if (event->key() == Qt::Key_Left) {

        }
        else if (event->key() == Qt::Key_Right) {

        }
        else if (event->key() != Qt::NoModifier) {
            int idx = getSelected();
            if (idx != -1) {
                Rom r = roms.at(idx);

                QString prog = path_prog + " " + prog_args;
                prog.replace("$(name)", r.name);

                // execute and WAIT until finished!
                int res = QProcess::execute(prog);
                if (res) {
                    qWarning() << "WARNING:" << prog;
                    switch (res) {
                    case -1:
                        qWarning() << "         Program crashed!";
                        break;

                    case -2:
                        qWarning() << "         Can't start program!";
                        break;

                    default:
                        qWarning() << "         Exits with code: "  << res;
                        break;
                    }
                }
            }
        }
    }
}

void Widget::paintEvent(QPaintEvent */*parent*/)
{
    QPainter painter(this);
    painter.fillRect(rect(), QBrush(Qt::black));

    QRectF r(view->x(), view->y() + getViewY(), view->width(), HEIGHT);
    painter.setPen(QPen(Qt::green, 4));
    painter.drawRoundedRect(r, 15, 15);

    QFont f("Verdana");
    f.setPixelSize(30);
    f.setBold(true);
    QFontMetrics m(f);

    painter.rotate(90);
    painter.translate(30, -20);
    painter.setPen(Qt::darkGray);
    painter.setOpacity(0.4);
    painter.setFont(f);
    painter.drawText(0, 0, romsView.at(romsIdx)->description());
}

void Widget::resizeEvent(QResizeEvent *event)
{
    globalW = width() * 0.6;

    // set the scrolling window
    if (view) {
        delete view;
        view = NULL;
    }
    view = new QGraphicsView();
    view->setParent(this);
    view->setDragMode(QGraphicsView::NoDrag);
    view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    view->setFrameShape(QFrame::NoFrame);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    view->setCacheMode(QGraphicsView::CacheBackground);
    view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    view->setStyleSheet("background: transparent");
    view->setFocusPolicy(Qt::NoFocus);
    view->move(50, 20);
    view->resize(globalW, height() - 40);

    // Game snapshot window
    snap->setGeometry(view->x() + view->width() + 20, view->y(),
                      width() - view->x() - view->width() - 40, view->height());
    snap->hide();

    // letter
    letter->raise();

    // do some intro animation!
    timer->setDuration(750);
    timer->setFrameRange(-view->height() / 2, 0);
    timer->setDirection(QTimeLine::Forward);
    timer->start();

    // setup what to display
    romsIdx = 1;
    selected = -1;
    setView();

    // ... and go!
    view->setSceneRect(0, -view->height() / 2, view->width(), view->height());
    view->show();
}


void Widget::nextAnimate(int frame)
{
    selected = frame;

    if (selected < 0 && timer->duration() == TIMER)
        selected = 0;

    if (selected + getViewY() - HEIGHT * 7 > scene->height())
        selected = scene->height() - getViewY() + HEIGHT * 7;

    view->setSceneRect(0, selected, view->width(), view->height());
}

void Widget::finishedAnimate()
{
    timer->setDuration(TIMER);

    if (keyPressed) {
        if (more < 100) more *= 2;

        if (timer->direction() == QTimeLine::Forward) {
            timer->setFrameRange(selected, selected + HEIGHT * more);
        }
        else {
            timer->setFrameRange(selected - HEIGHT * more, selected);
        }
        timer->start();
    }
    else {
        snap->setSnap(getSelected());
    }

    if (more > 20 || letter->haveLetter()) {
        letter->setLetter(romsView.at(romsIdx)->letter(getSelected()));
        if (!more) {
            letter->setLetter("");
        }
    }
}

void Widget::setView()
{
    // Catch the old position, if any!
    RomItem *prevItem = NULL;
    int prevSelected = -1;
    if (selected != -1) prevSelected = getSelected();

    // Allocate a new scene
    if (scene) {
        delete scene;
        scene = NULL;
    }
    scene = new QGraphicsScene();
    view->setScene(scene);

    // title of the scene
    // ... and populate it!
    int off = getViewY();
    for (int i = 0; i < romsView.at(romsIdx)->list.size(); i++) {
        RomItem *ri = new RomItem(romsView.at(romsIdx)->list.at(i));
        ri->setPos(0, off);
        scene->addItem(ri);

        off += ri->boundingRect().height();

        if (prevSelected == ri->getRom()) prevItem = ri;
    }

    more = 0;
    if (prevItem) {
        selected = prevItem->y() - getViewY();
    }
    else {
        selected = 0;
    }
    view->setSceneRect(0, selected, view->width(), view->height());

    // set viewers ...
    letter->setMe(view->x() + view->width(), view->y(), romsView.at(romsIdx)->letterSize());
    letter->hide();

    update();
}

int Widget::getViewY()
{
    return HEIGHT * ((view->height() / HEIGHT) / 2);
}

int Widget::getSelected()
{
    RomItem *r = (RomItem*) scene->itemAt(0, selected + getViewY());
    if (r)
        return r->getRom();
    else
        return -1;
}
