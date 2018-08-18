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

MarkdownEntry::MarkdownEntry(Worksheet* worksheet) : TextEntry(worksheet), rendered(false)
{
    m_textItem->installEventFilter(this);
}

MarkdownEntry::~MarkdownEntry()
{
}

void MarkdownEntry::setContent(const QString& content)
{
    rendered = false;
    plain = content;
    TextEntry::setContent(content);
}

void MarkdownEntry::setContent(const QDomElement& content, const KZip& file)
{
    Q_UNUSED(file);

    rendered = content.attribute(QLatin1String("rendered"), QLatin1String("1")) == QLatin1String("1");
    QDomElement htmlEl = content.firstChildElement(QLatin1String("HTML"));
    if(!htmlEl.isNull())
        html = htmlEl.text();
    else
    {
        html = QLatin1String("");
        rendered = false; // No html provided. Assume that it hasn't been rendered.
    }
    QDomElement plainEl = content.firstChildElement(QLatin1String("Plain"));
    if(!plainEl.isNull())
        plain = plainEl.text();
    else
    {
        plain = QLatin1String("");
        html = QLatin1String(""); // No plain text provided. The entry shouldn't render anything, or the user can't re-edit it.
    }
    if(rendered)
        m_textItem->setHtml(html);
    else
        m_textItem->setPlainText(plain);
}

QDomElement MarkdownEntry::toXml(QDomDocument& doc, KZip* archive)
{
    Q_UNUSED(archive);

    if(!rendered)
        plain = m_textItem->toPlainText();

    QDomElement el = doc.createElement(QLatin1String("Markdown"));
    el.setAttribute(QLatin1String("rendered"), (int)rendered);

    QDomElement plainEl = doc.createElement(QLatin1String("Plain"));
    plainEl.appendChild(doc.createTextNode(plain));
    el.appendChild(plainEl);
    if(rendered)
    {
        QDomElement htmlEl = doc.createElement(QLatin1String("HTML"));
        htmlEl.appendChild(doc.createTextNode(html));
        el.appendChild(htmlEl);
    }
    return el;
}

bool MarkdownEntry::evaluate(EvaluationOption evalOp)
{
    if(!rendered)
    {
        plain = m_textItem->toPlainText();
        rendered = renderMarkdown(plain);
    }

    evaluateNext(evalOp);
    return true;
}

bool MarkdownEntry::renderMarkdown(QString& plain)
{
#ifdef Discount_FOUND
    QByteArray mdCharArray = plain.toUtf8();
    MMIOT* mdHandle = mkd_string(mdCharArray.data(), mdCharArray.size()+1, 0);
    if(!mkd_compile(mdHandle, MKD_NOSUPERSCRIPT | MKD_FENCEDCODE | MKD_GITHUBTAGS))
    {
        qDebug()<<"Failed to compile the markdown document";
        mkd_cleanup(mdHandle);
        return false;
    }
    char *htmlDocument;
    int htmlSize = mkd_document(mdHandle, &htmlDocument);
    html = QString::fromUtf8(htmlDocument, htmlSize);
    mkd_cleanup(mdHandle);

    m_textItem->setHtml(html);
    return true;
#else
    Q_UNUSED(plain);

    return false;
#endif
}

bool MarkdownEntry::eventFilter(QObject* object, QEvent* event)
{
    if(object == m_textItem)
    {
        if(event->type() == QEvent::GraphicsSceneMouseDoubleClick)
        {
            QGraphicsSceneMouseEvent* mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent*>(event);
            if(!mouseEvent) return false;
            if(mouseEvent->button() == Qt::LeftButton && rendered)
            {
                QTextDocument* doc = m_textItem->document();
                doc->setPlainText(plain);
                m_textItem->setDocument(doc);
                m_textItem->setCursorPosition(mouseEvent->pos());
                m_textItem->textCursor().clearSelection();
                rendered = false;
                return true;
            }
        }
    }
    return false;
}

bool MarkdownEntry::wantToEvaluate()
{
    return !rendered;
}
