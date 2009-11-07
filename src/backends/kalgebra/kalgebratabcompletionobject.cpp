/*************************************************************************************
*  Copyright (C) 2009 by Aleix Pol <aleixpol@kde.org>                               *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#include "kalgebratabcompletionobject.h"

#include <QStringList>

#include "kalgebrasession.h"
#include <analitzagui/operatorsmodel.h>

KAlgebraTabCompletionObject::KAlgebraTabCompletionObject(const QString& command, KAlgebraSession* session)
    : Cantor::TabCompletionObject(command, session)
{
    //We only want identifiers
    int i;
    for(i=command.size()-1; i>=0 && command[i].isLetter(); i--) {}
    
    if(i>=0)
        setCommand(command.mid(i+1));
}

KAlgebraTabCompletionObject::~KAlgebraTabCompletionObject()
{}

void KAlgebraTabCompletionObject::fetchCompletions()
{
    OperatorsModel* opm=static_cast<KAlgebraSession*>(session())->operatorsModel();
    
    QModelIndexList idxs=opm->match(opm->index(0,0), Qt::DisplayRole, command(), 5, Qt::MatchStartsWith);
    QStringList comp;
    foreach(const QModelIndex& idx, idxs)
        comp << idx.data().toString();
    
    setCompletions(comp);
    emit done();
}