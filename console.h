#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>
#include <QTextBrowser>

#include "dataobject.h"

class DataSetObject;

class console : public QTextBrowser
{
    Q_OBJECT

public:
    console(QWidget *parent = 0);

    void setConsoleInput(QPlainTextEdit *in);
    void setDataSet(DataSetObject *dsobj);

public slots:
    void selectCommand(QStringList *args);

    void command(int i);

private:
    QPlainTextEdit *console_input;
    DataSetObject *dataSet;
};

#endif // CONSOLE_H
