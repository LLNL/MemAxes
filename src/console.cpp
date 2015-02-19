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

#include <QTime>

#include "console.h"

static QString titleText(
    "---- MemAxes Console ----\n"
);

static QString helpText(
    "Commands : \n"
    "    \n"
    "    select [--mode={new,append,filter}] <query>\n"
    "    hide <query>\n"
    "    show <query>\n"
    "    \n"
    "        <query> is of the form:\n"
    "           [DIMRANGE dim=vmin:vmax]\n"
    "           [RESOURCE resource=id]\n"
    "    \n"
    "    inspect\n"
    "    \n"
    "    derivedim <expression>\n"
    "        <expression> is of the form:\n"
    "            dim1 <op> dim2\n"
    "        <op> is one of:\n"
    "            + - * /\n"
    "Examples : \n"
    "    select DIMRANGE 4=30:40 5=4:5\n"
    "    \n"
//    "    select RESOURCE cpu=4 cache=L3\n"
);

console::console(QWidget *parent) :
    QTextBrowser(parent)
{
    dataSet = NULL;
    sb = this->verticalScrollBar();

    this->setFont(QFont("Consolas"));
    this->setReadOnly(true);

    log(titleText);
    helpCommand(NULL);
}

void console::setConsoleInput(QPlainTextEdit *in)
{
    console_input = in;
    console_input->setLayoutDirection(Qt::LeftToRight);
    connect(console_input,SIGNAL(blockCountChanged(int)),this,SLOT(command(int)));
}

void console::setDataSet(DataObject *dsobj)
{
    dataSet = dsobj;
}

void console::helpCommand(QStringList *args)
{
    Q_UNUSED(args);
    log(helpText);
}

void console::inspectCommand(QStringList *args)
{
    Q_UNUSED(args);

    // Print out some info about the current selection
    int numSel = dataSet->numSelected;
    int numTot = dataSet->numElements;

    log("Selected Samples : ");
    log(QString::number(numSel));
    log("Total Samples : ");
    log(QString::number(numTot));
}

void console::selectCommand(QStringList *args)
{
    if(dataSet == NULL)
    {
        log("Unable to select from the void, please load data first");
        return;
    }

    if(args == NULL || args->size() < 3)
    {
        log("Invalid arguments");
        return;
    }

    QUERY_TYPE qt = getQueryType(args->at(1));

    if(qt == QUERY_UNKNOWN)
    {
        log("Invalid arguments");
        return;
    }

    if(qt == QUERY_DIMRANGE)
    {
        struct dimRangeQuery drq = createDimRangeQuery(args);
        //dataSet->selectByMultiDimRange(drq.dims,drq.mins,drq.maxes);
        emit selectionChangedSig();
        return;
    }
}

CMD_TYPE console::getCommandType(QString cmd)
{
    cmd = cmd.toLower();
    if(cmd == "help" || cmd == "h")
        return CMD_HELP;
    else if(cmd == "select" || cmd == "sel")
        return CMD_SELECT;
    else if(cmd == "inspect" || cmd == "ins")
        return CMD_INSPECT;
    return CMD_UNKNOWN;
}

QUERY_TYPE console::getQueryType(QString qtype)
{
    qtype = qtype.toLower();
    if(qtype == "dimrange")
        return QUERY_DIMRANGE;
    else if(qtype == "resource")
        return QUERY_RESOURCE;
    return QUERY_UNKNOWN;
}

dimRange console::createDimRange(QString str)
{
    QStringList eqSplit = str.split("=");
    QStringList rangeStrs = eqSplit[1].split(":");

    struct dimRange dr;
    dr.dim = eqSplit[0];
    dr.min = rangeStrs[0].toDouble();
    dr.max = rangeStrs[1].toDouble();

    return dr;
}

struct dimRangeQuery console::createDimRangeQuery(QStringList *args)
{
    struct dimRangeQuery drq;

    for(int i=2; i<args->size(); i++)
    {
        struct dimRange dr = createDimRange(args->at(i));
        drq.dims.push_back(dr.dim);
        drq.mins.push_back(dr.min);
        drq.maxes.push_back(dr.max);
    }

    return drq;
}

void console::command(int i)
{
    Q_UNUSED(i);

    QString cmdLine = console_input->toPlainText().simplified();
    if(cmdLine.isEmpty())
        return;

    console_input->clear();
    console_input->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);

    QStringList cmdArgs = cmdLine.split(" ");
    QString cmd = cmdArgs.first();
    QString printCmdLine("$ "+cmdLine);
    log(printCmdLine);

    CMD_TYPE cmdType = getCommandType(cmd);
    switch(cmdType)
    {
    case(CMD_HELP):
        helpCommand(&cmdArgs);
        break;
    case(CMD_SELECT):
        selectCommand(&cmdArgs);
        break;
    case(CMD_INSPECT):
        inspectCommand(&cmdArgs);
        break;
    default:
        log("Command unrecognized, type 'help' or 'h' for a list of commands");
        break;
    }
}

void console::log(const char *msg)
{
    log(QString(msg));
}

void console::log(QString msg)
{
    QString timestamp = QTime::currentTime().toString();
    this->append(timestamp+"$ "+msg);

    // Scroll to bottom
    sb->setValue(sb->maximum());
}
