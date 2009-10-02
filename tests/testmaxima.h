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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#ifndef _TESTMAXIMA_H
#define _TESTMAXIMA_H

#include <QtTest>
#include <QtCore>

namespace Cantor
{
    class Session;
    class Expression;
}


/** This class test some of the basic functions of the maxima backend
    The different tests represent some general expressions, as well
    as expressions, that are known to have caused problems in earlier
    versions
**/
class TestMaxima : public QObject
{
  Q_OBJECT

public:
    TestMaxima();

private slots:
    //tests evaluating a simple command
    void testSimpleCommand();
    //tests a command, containing more than 1 line
    void testMultilineCommand();
    //tests doing a plot
    void testPlot();
    
    //tests a syntax error (not closing bracket)
    void testInvalidSyntax();
    //tests if the expression numbering works
    void testExprNumbering();
private:
    Cantor::Session* createSession();
    Cantor::Expression* evalExp(const QString& exp);

private:
    Cantor::Session* m_session;
};

#endif /* _TESTMAXIMA_H */