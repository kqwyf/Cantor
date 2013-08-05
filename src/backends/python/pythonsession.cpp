/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2012 Filipe Saraiva <filipe@kde.org>
 */

#include "pythonsession.h"
#include "pythonexpression.h"
#include "pythonhighlighter.h"
#include "pythoncompletionobject.h"
#include "pythonkeywords.h"

#include <kdebug.h>
#include <KDirWatch>

#include <QtCore/QFile>
#include <QTextEdit>
#include <QListIterator>
#include <QDir>
#include <QIODevice>
#include <QByteArray>
#include <QStringList>

#include <settings.h>
#include <qdir.h>

#include <string>

using namespace std;

PythonSession::PythonSession( Cantor::Backend* backend) : Session(backend)
{
    kDebug();
}

PythonSession::~PythonSession()
{
    kDebug();
}

void PythonSession::login()
{
    kDebug()<<"login";

    Py_Initialize();
    m_pModule = PyImport_AddModule("__main__");

    emit ready();
}

void PythonSession::logout()
{
    kDebug()<<"logout";

    QDir removePlotFigures;
    QListIterator<QString> i(m_listPlotName);

    while(i.hasNext()){
        removePlotFigures.remove(i.next().toLocal8Bit().constData());
    }

    changeStatus(Cantor::Session::Done);
}

void PythonSession::interrupt()
{
    kDebug()<<"interrupt";

    foreach(Cantor::Expression* e, m_runningExpressions)
        e->interrupt();

    m_runningExpressions.clear();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression* PythonSession::evaluateExpression(const QString& cmd, Cantor::Expression::FinishingBehavior behave)
{
    kDebug() << "evaluating: " << cmd;
    PythonExpression* expr = new PythonExpression(this);

    changeStatus(Cantor::Session::Running);

    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void PythonSession::runExpression(PythonExpression* expr)
{
    kDebug() << "run expression";

    QString command;

    command += expr->command();

    QStringList commandLine = command.split("\n");

    QString commandProcessing;

    for(int contLine = 0; contLine < commandLine.size(); contLine++){

        if(commandLine.at(contLine).contains("import ")){

            if(this->identifyKeywords(commandLine.at(contLine).simplified())){
                continue;
            } else {
                this->readOutput(expr, commandLine.at(contLine).simplified());
                return;
            }
        }

        if((!commandLine.at(contLine).contains("import ")) && (!commandLine.at(contLine).contains("=")) &&
           (!commandLine.at(contLine).contains("print"))   && (!commandLine.at(contLine).endsWith(":")) &&
           (!commandLine.at(contLine).startsWith(" "))){

            commandProcessing += "print " + commandLine.at(contLine) + "\n";

            continue;
        }

        if(commandLine.at(contLine).startsWith(" ")){

            if((commandLine.at(contLine).contains("import ")) || (commandLine.at(contLine).contains("=")) ||
               (commandLine.at(contLine).contains("print"))   || (commandLine.at(contLine).endsWith(":"))){

                commandProcessing += commandLine.at(contLine) + "\n";

                continue;
            }

            int contIdentationSpace;

            for(contIdentationSpace = 0; commandLine.at(contLine).at(contIdentationSpace).isSpace(); contIdentationSpace++);

            kDebug() << "contIdentationSpace: " << contIdentationSpace;

            QString commandIdentation;

            commandIdentation = commandLine.at(contLine);

            kDebug() << "Insert print in " << contIdentationSpace << "space";
            kDebug() << "commandIdentation before insert " << commandIdentation;

            commandIdentation.insert(contIdentationSpace, QString("print "));

            kDebug() << "commandIdentation after insert" << commandIdentation;

            commandProcessing += commandIdentation + "\n";

            continue;
        }

        commandProcessing += commandLine.at(contLine) + "\n";

    }

    this->readOutput(expr, commandProcessing);
}

void PythonSession::runClassOutputPython()
{
    QString classOutputPython = "import sys\n"                  \
                                "class CatchOut:\n"             \
                                "    def __init__(self):\n"     \
                                "        self.value = ''\n"     \
                                "    def write(self, txt):\n"   \
                                "        self.value += txt\n"   \
                                "output = CatchOut()\n"         \
                                "sys.stdout = output\n"         \
                                "sys.stderr = output\n\n";

    PyRun_SimpleString(classOutputPython.toStdString().c_str());
}

QString PythonSession::getPythonCommandOutput(QString commandProcessing)
{
    kDebug() << "Running python command" << commandProcessing.toStdString().c_str();

    this->runClassOutputPython();
    PyRun_SimpleString(commandProcessing.toStdString().c_str());

    PyObject *outputPython = PyObject_GetAttrString(m_pModule, "output");
    PyObject *output = PyObject_GetAttrString(outputPython, "value");
    string outputString = PyString_AsString(output);

    return QString(outputString.c_str());
}

bool PythonSession::identifyKeywords(QString command)
{
    QString verifyErrorImport;

    QString listKeywords;
    QString keywordsString;

    QString moduleImported;
    QString moduleVariable;

    verifyErrorImport = this->getPythonCommandOutput(command);

    kDebug() << "verifyErrorImport: " << verifyErrorImport;

    if((verifyErrorImport.contains("ImportError: ")) ||
        (verifyErrorImport.contains("SyntaxError: "))){

        kDebug() << "returned false";

        return false;
    }

    moduleImported += this->identifyPythonModule(command);
    moduleVariable += this->identifyVariableModule(command);

    if((moduleVariable.isEmpty()) && (!command.endsWith("*"))){
        keywordsString = command.section(" ", 3).remove(" ");
    }

    if(moduleVariable.isEmpty() && (command.endsWith("*"))){
        listKeywords += "import " + moduleImported + "\n"    \
                        "print dir(" + moduleImported + ")\n";
    }

    if(!moduleVariable.isEmpty()){
        listKeywords += "print dir(" + moduleVariable + ")\n";
    }

    if(!listKeywords.isEmpty()){
        keywordsString = QString(this->getPythonCommandOutput(listKeywords));

        keywordsString.remove("'");
        keywordsString.remove(" ");
        keywordsString.remove("[");
        keywordsString.remove("]");
    }

    kDebug() << "keywordsString" << keywordsString;

    QStringList keywordsList = keywordsString.split(",");

    kDebug() << "keywordsList" << keywordsList;

    PythonKeywords::instance()->loadFromModule(moduleVariable, keywordsList);

    kDebug() << "Module imported" << moduleImported;

    return true;
}

QString PythonSession::identifyPythonModule(QString command)
{
    QString module;

    if(command.contains("import ")){
        module = command.section(" ", 1, 1);
    }

    kDebug() << "module identified" << module;
    return module;
}

QString PythonSession::identifyVariableModule(QString command)
{
    QString variable;

    if(command.contains("import ")){
        variable = command.section(" ", 1, 1);
    }

    if((command.contains("import ")) && (command.contains(" as "))){
        variable = command.section(" ", 3, 3);
    }

    if(command.contains("from ")){
        variable = "";
    }

    kDebug() << "variable identified" << variable;
    return variable;
}

void PythonSession::expressionFinished()
{
    kDebug()<<"finished";
    PythonExpression* expression = qobject_cast<PythonExpression*>(sender());

    m_runningExpressions.removeAll(expression);
    kDebug() << "size: " << m_runningExpressions.size();
}

void PythonSession::readOutput(PythonExpression* expr, QString commandProcessing)
{
    kDebug() << "readOutput";

    m_output = QString(this->getPythonCommandOutput(commandProcessing));

    expr->parseOutput(m_output);
    expr->evalFinished();

    changeStatus(Cantor::Session::Done);

    kDebug() << "output: " << m_output;
}

// void PythonSession::plotFileChanged(QString filename)
// {
//     kDebug() << "plotFileChanged filename:" << filename;
//
//     if ((m_currentExpression) && (filename.contains("cantor-export-scilab-figure")))
//     {
//          kDebug() << "Calling parsePlotFile";
//          m_currentExpression->parsePlotFile(filename);
//
//          m_listPlotName.append(filename);
//     }
// }

void PythonSession::currentExpressionStatusChanged(Cantor::Expression::Status status)
{
    kDebug() << "currentExpressionStatusChanged: " << status;

    switch (status)
    {
        case Cantor::Expression::Computing:
            break;

        case Cantor::Expression::Interrupted:
            break;

        case Cantor::Expression::Done:
        case Cantor::Expression::Error:
            changeStatus(Done);

            break;
    }
}

QSyntaxHighlighter* PythonSession::syntaxHighlighter(QObject* parent)
{
    return new PythonHighlighter(parent);
}

Cantor::CompletionObject* PythonSession::completionFor(const QString& command, int index)
{
    return new PythonCompletionObject(command, index, this);
}

#include "pythonsession.moc"
