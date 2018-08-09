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
	Copyright (C) 2018 Yifei Wu <kqwyfg@gmail.com>
 */

#include "lib/markdown.h"
#include <sstream>

#include "markdownentry.h"

#include <QDebug>

MarkdownEntry::MarkdownEntry(Worksheet* worksheet) : TextEntry(worksheet), dirty(false), evalJustNow(true)
{
	m_textItem->installEventFilter(this);
}

MarkdownEntry::~MarkdownEntry()
{
}

bool MarkdownEntry::evaluate(EvaluationOption evalOp)
{
	if(m_textItem->hasFocus()) // text in the entry may be edited
		plain = m_textItem->toPlainText();

	QString t_markdown(plain);
	t_markdown.replace(QLatin1String("\n"), QLatin1String("\n\n")); // a blank line results in a <p> in html

	// convert markdown to html
    markdown::Document document;
    document.read(t_markdown.toStdString());
    std::ostringstream htmlStream;
    document.write(htmlStream);
	html = QString::fromStdString(htmlStream.str());

	m_textItem->setHtml(html);
	dirty = false;
	evalJustNow = true;
	return TextEntry::evaluate(evalOp);
}

bool MarkdownEntry::eventFilter(QObject* object, QEvent* event)
{
	if(object == m_textItem)
	{
		if(event->type() == QEvent::FocusIn)
		{
			QString plainHtml = QLatin1String("<p>") + plain + QLatin1String("</p>"); // clear the style, such as font
			plainHtml.replace(QLatin1String("\n"), QLatin1String("<br>"));
			m_textItem->setHtml(plainHtml); 
		}
		else if(event->type() == QEvent::FocusOut)
		{
			if(evalJustNow)
			{
				evalJustNow = false;
				return false;
			}

			if(!dirty && plain.compare(m_textItem->toPlainText()) == 0)
			{
				m_textItem->setHtml(html);
				TextEntry::evaluate(WorksheetEntry::FocusNext);
			}
			else
			{
				dirty = true;
				plain = m_textItem->toPlainText();
			}
		}
	}
	return false;
}
