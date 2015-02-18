//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. Written by Alfredo
// Gimenez (alfredo.gimenez@gmail.com). LLNL-CODE-663358. All rights
// reserved.
//
// This file is part of MemAxes. For details, see
// https://github.com/scalability-tools/MemAxes
//
// Please also read this link – Our Notice and GNU Lesser General Public
// License. This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License (as
// published by the Free Software Foundation) version 2.1 dated February
// 1999.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the IMPLIED WARRANTY OF
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms and
// conditions of the GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// OUR NOTICE AND TERMS AND CONDITIONS OF THE GNU GENERAL PUBLIC LICENSE
// Our Preamble Notice
// A. This notice is required to be provided under our contract with the
// U.S. Department of Energy (DOE). This work was produced at the Lawrence
// Livermore National Laboratory under Contract No. DE-AC52-07NA27344 with
// the DOE.
// B. Neither the United States Government nor Lawrence Livermore National
// Security, LLC nor any of their employees, makes any warranty, express or
// implied, or assumes any liability or responsibility for the accuracy,
// completeness, or usefulness of any information, apparatus, product, or
// process disclosed, or represents that its use would not infringe
// privately-owned rights.
//////////////////////////////////////////////////////////////////////////////

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QScrollBar>

#include "dataobject.h"
#include "util.h"

class DataObject;

enum CMD_TYPE {
    CMD_HELP = 0,
    CMD_SELECT,
    CMD_INSPECT,
    CMD_UNKNOWN
};

enum QUERY_TYPE {
    QUERY_DIMRANGE = 0,
    QUERY_RESOURCE,
    QUERY_UNKNOWN
};

struct dimRangeQuery {
    QVector<QString> dims;
    QVector<qreal> mins;
    QVector<qreal> maxes;
};

struct dimRange {
    QString dim;
    qreal min;
    qreal max;
};

class console : public QTextBrowser
{
    Q_OBJECT

public:
    console(QWidget *parent = 0);

    void setConsoleInput(QPlainTextEdit *in);
    void setDataSet(DataObject *dsobj);

signals:
    void selectionChangedSig();

public slots:
    CMD_TYPE getCommandType(QString cmd);
    QUERY_TYPE getQueryType(QString qtype);

    struct dimRange createDimRange(QString str);
    struct dimRangeQuery createDimRangeQuery(QStringList *args);

    void helpCommand(QStringList *args);
    void inspectCommand(QStringList *args);
    void selectCommand(QStringList *args);

    void command(int i);
    void log(const char *msg);
    void log(QString msg);

private:
    QPlainTextEdit *console_input;
    DataObject *dataSet;
    QScrollBar *sb;
};

#endif // CONSOLE_H
