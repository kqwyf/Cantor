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

#include "textentry.h"
#include "worksheettextitem.h"

#include <QDebug>
#include <KLocalizedString>

#include <iostream>
#include <sstream>

MarkdownEntry::MarkdownEntry(Worksheet* worksheet) : WorksheetEntry(worksheet), m_textItem(new WorksheetTextItem(this, Qt::TextEditorInteraction))
{
    m_textItem->enableRichText(true);
    connect(m_textItem, &WorksheetTextItem::moveToPrevious, this, &MarkdownEntry::moveToPreviousEntry);
    connect(m_textItem, &WorksheetTextItem::moveToNext, this, &MarkdownEntry::moveToNextEntry);
    connect(m_textItem, SIGNAL(execute()), this, SLOT(evaluate()));
    connect(m_textItem, &WorksheetTextItem::doubleClick, this, &MarkdownEntry::resolveImagesAtCursor);
}

MarkdownEntry::~MarkdownEntry()
{
}

bool MarkdownEntry::evaluate(EvaluationOption evalOp)
{
    std::string cstr = string((const char*)((d->data).toLocal8Bit()));
    std::stringbuf buf;
    buf.str(cstr);
    std::istream in(&buf);
    markdown::Document doc;
    doc.read(in);
    std::ostream out;
    doc.write(out);
    std::string ans = out.str();
    QString html=QString::fromLocal8Bit(ans.c_str());
	m_textItem->setHtml(html);
	return TextEntry::evaluate(evalOp);
}
