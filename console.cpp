#include "console.h"

static const char *helpText = {
    "---- MemAxes Console ----\n"
    "Commands : "
    "    select [--mode={new,append,filter}] <query>"
    "    hide <query>"
    "    show <query>"
    "        "
    "    <query> is of the form:"
    "           [DIMRANGE dim=vmin:vmax]"
    "           [RESOURCE resource=id]"
    "    "
    "Examples : "
    "    select DIMRANGE latency=30:40 cpu=4:5"
    "    select DIMRANGE dim6=20:200 dim8=40:45"
    "    "
    "    select RESOURCE socket=1"
    "    select RESOURCE cpu=4 cache=L3"
    "    select RESOURCE NUMA=2"
    "    "
};

console::console(QWidget *parent) :
    QTextBrowser(parent)
{
    QFont font("Consolas");
    this->setCurrentFont(font);
    this->setReadOnly(true);
    this->append(helpText);
}

void console::setConsoleInput(QPlainTextEdit *in)
{
    console_input = in;
    connect(console_input,SIGNAL(blockCountChanged(int)),this,SLOT(command(int)));
}

void console::setDataSet(DataSetObject *dsobj)
{
    dataSet = dsobj;
}

void console::selectCommand(QStringList *args)
{

}

void console::command(int i)
{
    Q_UNUSED(i);

    QString cmdLine = console_input->toPlainText().simplified();
    console_input->clear();

    QStringList cmdArgs = cmdLine.split(" ");

    QString cmd = cmdArgs.first();

    if(cmd == "select")
    {
        selectCommand(&cmdArgs);
    }

    this->append(cmdLine);
}
