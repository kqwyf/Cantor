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

#include "latexresult.h"
using namespace MathematiK;

#include <QFile>
#include <QTextStream>
#include <kdebug.h>

class MathematiK::LatexResultPrivate
{
  public:
    LatexResultPrivate()
    {
        showCode=false;
    }

    bool showCode;
    QString code;
};

LatexResult::LatexResult(const QString& code, const KUrl& url) : EpsResult( url ),
                                                                 d(new LatexResultPrivate)
{
    d->code=code;
}

LatexResult::~LatexResult()
{
    delete d;
}

int LatexResult::type()
{
    return LatexResult::Type;
}

QString LatexResult::mimeType()
{
    if(isCodeShown())
        return "text/plain";
    else
        return EpsResult::mimeType();
}

QString LatexResult::code()
{
    return d->code;
}

bool LatexResult::isCodeShown()
{
    return d->showCode;
}

void LatexResult::showCode()
{
    d->showCode=true;
}

void LatexResult::showRendered()
{
    d->showCode=false;
}

QVariant LatexResult::data()
{
    if(isCodeShown())
        return QVariant(code());
    else
        return EpsResult::data();
}

QString LatexResult::toHtml()
{
    if (isCodeShown())
    {
            QString s=code();
            s.replace('\n', "<br/>\n");
            return s;
    }
    else
    {
        return EpsResult::toHtml();
    }
}

QDomElement LatexResult::toXml(QDomDocument& doc)
{
    kDebug()<<"saving textresult "<<toHtml();
    QDomElement e=doc.createElement("Result");
    e.setAttribute("type", "latex");
    KUrl url=KUrl(EpsResult::data().toUrl());
    e.setAttribute("filename", url.fileName());
    QDomText txt=doc.createTextNode(code());
    e.appendChild(txt);

    return e;
}

void LatexResult::save(const QString& filename)
{
    if(isCodeShown())
    {
        QFile file(filename);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream stream(&file);

        stream<<code();

        file.close();
    }else
    {
        EpsResult::save(filename);
    }
}
