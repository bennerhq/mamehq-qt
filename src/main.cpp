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

#include <iostream>
using namespace std;

#include "widget.h"
#include "getroms.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QList>
#include <QtGui/QApplication>

// ----------------------------------------------------------------------------

QString path_total      = "$(path)data/listxml.xml";
QString path_verify     = "$(path)data/verifyromsets.txt";
QString path_marquees   = "$(path)marquees/";
QString path_snaps      = "$(path)snap/";
QString path_cabinets   = "$(path)cabinets/";
QString path_roms       = "$(path)roms/";
QString path_prog       = "$(path)gomame";
QString prog_args       = "$(name).zip";

// ----------------------------------------------------------------------------

QString path_curr;
QString path_home;
QList<QString> path_search;

QString initPath(QString path)
{
    bool found = false;

    if (path.indexOf("$(path)") == -1) {
        found = QFile::exists(path);
    }
    else {
        QString work;
        for (int i = 0; i < path_search.size(); i++) {
            work = path;
            work.replace("$(path)", path_search.at(i));
            if (QFile::exists(work)) {
                path = work;
                found = true;
                break;
            }
        }
    }

    if (!found) {
        qWarning() << "WARNING:" << path << " not found!";
    }

    return path;
}

// ----------------------------------------------------------------------------

void usage()
{
    cout << "usage:" << endl;
    cout << "    mamehq <arguments>" << endl;
    cout << "" << endl;
    cout << "arguments:" << endl;
    cout << "    -help                 this text" << endl;
    cout << "    -path                 base path for the below paths" << endl;
    cout << "    -romspath <path>      path to roms. default " << path_roms.toStdString() << endl;
    cout << "    -snapspath <path>     path to game snapshots. default " << path_snaps.toStdString() << endl;
    cout << "    -cabinetspath <path>  path to cabinets pictures. default " << path_cabinets.toStdString() << endl;
    cout << "    -marqueespath <path>  path to marquees pictures. default " << path_marquees.toStdString() << endl;
    cout << "    -progpath <filename>  program to exectue when starting a rom. default " << path_prog.toStdString() << endl;
    cout << "    -progargs <path>      arguments to -progpath. default " << prog_args.toStdString() << endl;
    cout << "    -total <filename>     generated with 'mame -listxml'. default is " << path_total.toStdString() << endl;
    cout << "    -verify <filename>    generated with 'mame -verifyromsets'. default is " << path_verify.toStdString() << endl;
    cout << endl;
    cout << "where:" << endl;
    cout << "    $(path)               is '-path'" << endl;
    cout << "    $(name)               is current rom name" << endl;
}

// ----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // program path
    path_curr = QDir::currentPath() + "/";
    path_curr.replace("//", "/");

    // user home path
    path_home = QDir::homePath() + "/";
    path_home.replace("//", "/");

    // setup default search path
    path_search += path_curr;
    path_search += path_home;
    path_search += path_home + "mame/";
    path_search += path_home + "mamehq/";
    path_search += path_home + "Programming/mamehq/";

    // program arguments ...
    for (int i = 1; i < argc; i ++) {
        QString cmd = argv[i];
        if (cmd == "-path" && i + 1 < argc) {
            path_search.clear();
            path_search += argv[i ++];
        }
        else if (cmd == "-romspath" && i + 1 < argc) {
            path_roms = argv[i ++];
        }
        else if (cmd == "-snapspath" && i + 1 < argc) {
            path_snaps = argv[i ++];
        }
        else if (cmd == "-cabinetspath" && i + 1 < argc) {
            path_cabinets = argv[i ++];
        }
        else if (cmd == "-marqueespath" && i + 1 < argc) {
            path_marquees = argv[i ++];
        }
        else if (cmd == "-progpath" && i + 1 < argc) {
            path_prog = argv[i ++];
        }
        else if (cmd == "-progargs" && i + 1 < argc) {
            prog_args = argv[i ++];
        }
        else if (cmd == "-total" && i + 1 < argc) {
            path_total = argv[i ++];
        }
        else if (cmd == "-verify" && i + 1 < argc) {
            path_verify = argv[i ++];
        }
        else {
            if (cmd != "-help") {
                qWarning() << "Unknown parameter: " << argv[i];
                qWarning() << "";
            }
            usage();
            return -1;
        }
    }

    // initiate pathes
    path_total = initPath(path_total);
    path_verify = initPath(path_verify);
    path_marquees = initPath(path_marquees);
    path_cabinets = initPath(path_cabinets);
    path_snaps = initPath(path_snaps);
    path_roms = initPath(path_roms);
    path_prog = initPath(path_prog);
    prog_args.replace("$(name)", path_roms + "$(name)");

    // read roms definitions!
    readFiles();
    if (!romsView.size() || !romsView.at(0)->list.size()) {
        qWarning() << "ERROR: " << path_total
                   << " and " << path_verify
                   << " have no valid roms!";
        return -1;
    }

    // go program!
    Widget w;
    w.show();

    return a.exec();
}
