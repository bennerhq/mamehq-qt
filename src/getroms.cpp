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

// ----------------------------------------------------------------------------

#include "getroms.h"

#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>

// ----------------------------------------------------------------------------

QList<Rom> roms;
QMap<QString, int> romsName;
QList<RomsView*> romsView;

// ----------------------------------------------------------------------------

bool romYearLessThan(const int &s1, const int &s2)
{
    return roms.at(s1).year + roms.at(s1).description <
           roms.at(s2).year + roms.at(s2).description;
}

bool romDescriptionLessThan(const int &s1, const int &s2)
{
    return roms.at(s1).description < roms.at(s2).description;
}

bool romManufacturerLessThan(const int &s1, const int &s2)
{
    return roms.at(s1).manufacturer < roms.at(s2).manufacturer;
}

void readFiles() {
    QFile file(path_total);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == "game") {
                Rom r;

                r.name = xml.attributes().value("name").toString();
                r.description = "?";
                r.manufacturer = "?";
                r.state = STATE_UNKNOWN;
                r.year = "????";
                r.playcount = 0;

                while (xml.readNextStartElement()) {
                    if (xml.name() == "description") {
                        r.description = xml.readElementText().trimmed();
                    }
                    else if (xml.name() == "year") {
                        r.year = xml.readElementText().trimmed();
                    }
                    else if (xml.name() == "manufacturer") {
                        r.manufacturer = xml.readElementText().trimmed();
                    }
                }

                romsName.insert(r.name, roms.size());
                roms += r;
            }
        }
    }
    if (xml.hasError()) {
        qDebug() << "*** ERROR: " << xml.lineNumber()<<" / "<< xml.errorString();
        return;
    }

    QFile file2(path_verify);
    if (!file2.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    file2.readLine();
    file2.readLine();

    romsView += new RomsDesc; // 0: Broken
    romsView += new RomsDesc; // 1: Description
    romsView += new RomsYear; // 2: Year
    romsView += new RomsManu; // 3: Manuafacture

    while (!file2.atEnd()) {
        QString str = file2.readLine();
        QString name = str.mid(0, 8).trimmed();
        QString state = str.mid(9).trimmed();

        if (romsName.contains(name)) {
            int p1 = romsName.value(name);
            Rom r = roms.at(p1);
            if (state == "correct") {
                r.state = STATE_CORRECT;
            }
            else if (state == "incorrect") {
                r.state = STATE_INCORRECT;
            }
            else if (state == "not found") {
                r.state = STATE_NOT_FOUND;
            }
            else if (state == "best available") {
                r.state = STATE_BEST_AVAILABLE;
            }
            else {
                r.state = STATE_UNKNOWN;
            }
            roms[p1] = r;

            if (r.state == STATE_CORRECT || r.state == STATE_BEST_AVAILABLE) {
                romsView[1]->list += p1;
                romsView[2]->list += p1;
                romsView[3]->list += p1;
            }
            else {
                romsView[0]->list += p1;
            }
        }
    }
    file2.close();

    for (int j = 0; j < romsView.size(); j++) {
        romsView[j]->sort();
    }
}
