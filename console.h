#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QScrollBar>

#include "dataobject.h"
#include "util.h"

class DataSetObject;

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
    QVector<int> dims;
    QVector<qreal> mins;
    QVector<qreal> maxes;
};

struct dimRange {
    int dim;
    qreal min;
    qreal max;
};

class console : public QTextBrowser
{
    Q_OBJECT

public:
    console(QWidget *parent = 0);

    void setConsoleInput(QPlainTextEdit *in);
    void setDataSet(DataSetObject *dsobj);

signals:
    void selectionChangedSig();

public slots:
    CMD_TYPE getCommandType(QString cmd);
    QUERY_TYPE getQueryType(QString qtype);

    int dimFromString(QString dstr);

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
    DataSetObject *dataSet;
    QScrollBar *sb;
};

#endif // CONSOLE_H
