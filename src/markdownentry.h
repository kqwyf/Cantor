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
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#ifndef MARKDOWNENTRY_H
#define MARKDOWNENTRY_H

#include <QString>
#include <QDomElement>
#include <QDomDocument>
#include <QIODevice>
#include <KZip>
#include <QTextCursor>
#include <KArchive>


#include "worksheettextitem.h"
#include "textentry.h"

class MarkdownEntry : public TextEntry
{
  Q_OBJECT
  public:
    MarkdownEntry(Worksheet* worksheet);
    ~MarkdownEntry() override;

    enum {Type = UserType + 2};

  public Q_SLOTS:
    bool evaluate(WorksheetEntry::EvaluationOption evalOp = FocusNext) Q_DECL_OVERRIDE;
};

#endif //MARKDOWNENTRY_H
