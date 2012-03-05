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

#ifndef GETROMS_H
#define GETROMS_H

#include <QString>
#include <QList>
#include <QMap>
#include <QDebug>

// ----------------------------------------------------------------------------

extern QString path_total;
extern QString path_verify;
extern QString path_marquees;
extern QString path_snaps;
extern QString path_cabinets;
extern QString path_roms;
extern QString path_prog;
extern QString prog_args;

// ----------------------------------------------------------------------------

#define STATE_UNKNOWN           0
#define STATE_CORRECT           1
#define STATE_INCORRECT         2
#define STATE_NOT_FOUND         3
#define STATE_BEST_AVAILABLE    4

typedef struct {
    QString name;
    QString year;
    QString description;
    QString manufacturer;
    int state;

    int playcount;
} Rom;

extern QList<Rom> roms;
extern QMap<QString, int> romsName;

// ----------------------------------------------------------------------------

bool romYearLessThan(const int &s1, const int &s2);
bool romDescriptionLessThan(const int &s1, const int &s2);
bool romManufacturerLessThan(const int &s1, const int &s2);

class RomsView {
public:
    QList<int> list;

    virtual QString description() = 0;
    virtual QString letterSize() = 0;
    virtual QString letter(int i) = 0;
    virtual void sort() = 0;
};

class RomsDesc : public RomsView {
public:
    virtual QString description() {
        return "Game name";
    }

    virtual QString letterSize() {
        return "XX";
    }

    virtual QString letter(int idx) {
        QString txt = roms.at(idx).description.at(0).toUpper();
        if (txt.at(0) < QChar('A')) txt = "#";
        return txt;
    }

    virtual void sort() {
        qSort(list.begin(), list.end(), romDescriptionLessThan);
    }
};

class RomsManu : public RomsDesc {
public:
    virtual QString description() {
        return "Manufacturer name";
    }

    virtual QString letter(int idx) {
        QString txt = roms.at(idx).manufacturer.at(0).toUpper();
        if (txt.at(0) < QChar('A')) txt = "#";
        return txt;
    }

    virtual void sort() {
        qSort(list.begin(), list.end(), romDescriptionLessThan);
    }
};

class RomsYear : public RomsView {
public:
    virtual QString description() {
        return "Published year";
    }

    virtual QString letterSize() {
        return "'XX";
    }

    virtual QString letter(int idx) {
        QString txt = roms.at(idx).year.right(2);
        if (txt == "??") txt = "?"; else txt = "'" + txt;
        return txt;
    }

    virtual void sort() {
        qSort(list.begin(), list.end(), romYearLessThan);
    }
};

extern QList<RomsView*> romsView;

// ----------------------------------------------------------------------------

void readFiles();

#endif // GETROMS_H
