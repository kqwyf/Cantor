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

#ifndef _PYTHONEXPRESSION_H
#define _PYTHONEXPRESSION_H

#include "expression.h"
#include <QStringList>

class QTimer;

class PythonExpression : public Cantor::Expression
{
  Q_OBJECT
  public:
    PythonExpression( Cantor::Session* session);
    ~PythonExpression();

    void evaluate();
    void interrupt();
    void parseOutput(QString output);
    void parseError(QString error);
//     void parsePlotFile(QString filename);
//     void setPlotPending(bool plot);

  private:
    bool m_finished;
//     bool m_plotPending;
};

#endif /* _PYTHONEXPRESSION_H */
