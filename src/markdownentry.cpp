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

#include "markdownentry.h"

#include "config-cantor.h"

#ifdef Discount_FOUND
extern "C" {
#include <mkdio.h>
}
#endif

#include <QDebug>

MarkdownEntry::MarkdownEntry(Worksheet* worksheet) : TextEntry(worksheet), dirty(true), evalJustNow(false)
{
    m_textItem->installEventFilter(this);
}

MarkdownEntry::~MarkdownEntry()
{
}

void MarkdownEntry::setContent(const QString& content)
{
    dirty = true;
    plain = content;
    TextEntry::setContent(content);
}

void MarkdownEntry::setContent(const QDomElement& content, const KZip& file)
{
    Q_UNUSED(file);

    dirty = content.attribute(QLatin1String("dirty"), QLatin1String("1")) == QLatin1String("1");
    QDomElement htmlEl = content.firstChildElement(QLatin1String("HTML"));
    if(!htmlEl.isNull())
        html = htmlEl.text();
    else
    {
        html = QLatin1String("");
        dirty = true; // No html provided. Assume that it hasn't been evaluated.
    }
    QDomElement plainEl = content.firstChildElement(QLatin1String("Plain"));
    if(!plainEl.isNull())
        plain = plainEl.text();
    else
    {
        plain = QLatin1String("");
        html = QLatin1String(""); // No plain text provided. The entry shouldn't render anything, or the user can't re-edit it.
    }
    if(dirty || m_textItem->hasFocus())
        m_textItem->setPlainText(plain);
    else
        m_textItem->setHtml(html);
}

QDomElement MarkdownEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

    if(m_textItem->hasFocus()) // text in the entry may be edited
    {
        plain = m_textItem->toPlainText();
        dirty = true;
    }

    QDomElement el = doc.createElement(QLatin1String("Markdown"));
    el.setAttribute(QLatin1String("dirty"), (int)dirty);

    QDomElement plainEl = doc.createElement(QLatin1String("Plain"));
    plainEl.appendChild(doc.createTextNode(plain));
    el.appendChild(plainEl);
    if(!dirty && !html.isEmpty())
    {
        QDomElement htmlEl = doc.createElement(QLatin1String("HTML"));
        htmlEl.appendChild(doc.createTextNode(html));
        el.appendChild(htmlEl);
    }
    return el;
}

bool MarkdownEntry::evaluate(EvaluationOption evalOp)
{
    if(m_textItem->hasFocus())
    {
        plain = m_textItem->toPlainText(); // text in the entry may be edited
        evalJustNow = true; // used in FocusOut event
    }
    dirty = false;

#ifdef Discount_FOUND
    // convert markdown to html
    QByteArray mdCharArray = plain.toUtf8();
    MMIOT* mdHandle = mkd_string(mdCharArray.data(), mdCharArray.size()+1, 0); // get the size of the string in byte
    if(!mkd_compile(mdHandle, MKD_NOSUPERSCRIPT | MKD_FENCEDCODE | MKD_GITHUBTAGS))
    {
        qDebug()<<"Failed to compile the markdown document";
        mkd_cleanup(mdHandle);
        return TextEntry::evaluate(evalOp);
    }
    char *htmlDocument;
    int htmlSize = mkd_document(mdHandle, &htmlDocument);
    html = QString::fromUtf8(htmlDocument, htmlSize);
    mkd_cleanup(mdHandle);

    m_textItem->setHtml(html);
#endif
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
            if(evalJustNow) // avoid evaluating just after an evaluation
            {
                evalJustNow = false;
                return false;
            }

            if(!dirty && plain == m_textItem->toPlainText())
            {
#ifdef Discount_FOUND
                m_textItem->setHtml(html);
#else
                if(!html.isEmpty()) // the entry is loaded from Xml
                    m_textItem->setHtml(html);
                else
                    m_textItem->setPlainText(plain);
#endif
                TextEntry::evaluate(EvaluationOption::DoNothing); // render the latex code
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

bool MarkdownEntry::wantToEvaluate()
{
    return dirty;
}
