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

#include "expression.h"
#include "latexrenderer.h"
using namespace Cantor;

#include <config-cantorlib.h>

#include "session.h"
#include "result.h"
#include "textresult.h"
#include "imageresult.h"
#include "latexresult.h"
#include "settings.h"

#include <QFileInfo>
#include <QString>

#include <KProcess>
#include <QDebug>
#include <KZip>

class Cantor::ExpressionPrivate
{
public:
    ExpressionPrivate() : result(nullptr), status(Expression::Done), session(nullptr),
    finishingBehavior(Expression::DoNotDelete), isInternal(false)
    {
    }

    int id;
    QString command;
    QString error;
    QList<QString> information;
    Result* result;
    Expression::Status status;
    Session* session;
    Expression::FinishingBehavior finishingBehavior;
    bool isInternal;
};

static const QString tex=QLatin1String("\\documentclass[12pt,fleqn]{article}          \n "\
                         "\\usepackage{latexsym,amsfonts,amssymb,ulem}  \n "\
                         "\\usepackage[dvips]{graphicx}                 \n "\
                         "\\setlength\\textwidth{5in}                   \n "\
                         "\\setlength{\\parindent}{0pt}                 \n "\
                         "%1                                            \n "\
                         "\\pagestyle{empty}                            \n "\
                         "\\begin{document}                             \n "\
                         "%2                                            \n "\
                         "\\end{document}\n");


Expression::Expression( Session* session ) : QObject( session ),
                                             d(new ExpressionPrivate)
{
    d->session=session;
    d->id=session->nextExpressionId();
}

Expression::~Expression()
{
    delete d->result;
    delete d;
}

void Expression::setCommand(const QString& command)
{
    d->command=command;
}

QString Expression::command()
{
    return d->command;
}

void Expression::setErrorMessage(const QString& error)
{
    d->error=error;
}

QString Expression::errorMessage()
{
    return d->error;
}

void Expression::setResult(Result* result)
{
    if(result!=nullptr)
    {
        qDebug()<<"settting result to a type "<<result->type()<<" result";
        #ifdef WITH_EPS
        //If it's text, and latex typesetting is enabled, render it
        if ( session()->isTypesettingEnabled()&&
             result->type()==TextResult::Type &&
             dynamic_cast<TextResult*>(result)->format()==TextResult::LatexFormat &&
             !result->toHtml().trimmed().isEmpty() &&
             finishingBehavior()!=DeleteOnFinish &&
             !isInternal()
            )
        {
            renderResultAsLatex(result);
            return;
        }
        #endif
    }

    if(d->result)
        delete d->result;

    d->result=result;

    emit gotResult();
}

Result* Expression::result()
{
    return d->result;
}

void Expression::clearResult()
{
    if(d->result)
        delete d->result;

    d->result=nullptr;
}

void Expression::setStatus(Expression::Status status)
{
    d->status=status;
    emit statusChanged(status);

    if(status==Expression::Done&&d->finishingBehavior==Expression::DeleteOnFinish)
        deleteLater();
}

Expression::Status Expression::status()
{
    return d->status;
}

Session* Expression::session()
{
    return d->session;
}
void Expression::renderResultAsLatex(Result* result)
{
    qDebug()<<"rendering as latex";
    qDebug()<<"checking if it really is a formula that can be typeset";

    LatexRenderer* renderer=new LatexRenderer(this);
    renderer->setLatexCode(result->data().toString().trimmed());
    renderer->addHeader(additionalLatexHeaders());

    connect(renderer, &LatexRenderer::done, [=] { latexRendered(renderer, result); });
    connect(renderer, &LatexRenderer::error, [=] { latexRendered(renderer, result); });

    renderer->render();
}

void Expression::latexRendered(LatexRenderer* renderer, Result* result)
{
    qDebug()<<"rendered a result to "<<renderer->imagePath();
    //replace the textresult with the rendered latex image result
    //ImageResult* latex=new ImageResult( d->latexFilename );
    if(renderer->renderingSuccessful())
    {
        if (result->type() == TextResult::Type)
        {
            TextResult* r=dynamic_cast<TextResult*>(result);
            LatexResult* latex=new LatexResult(r->data().toString().trimmed(), QUrl::fromLocalFile(renderer->imagePath()), r->plain());
            setResult( latex );
        }
        else if (result->type() == LatexResult::Type)
        {
            LatexResult* previousLatexResult=dynamic_cast<LatexResult*>(result);
            LatexResult* latex=new LatexResult(previousLatexResult->data().toString().trimmed(), QUrl::fromLocalFile(renderer->imagePath()), previousLatexResult->plain());
            setResult( latex );
        }
    }else
    {
        //if rendering with latex was not successful, just use the plain text version
        //if available
        TextResult* r=dynamic_cast<TextResult*>(result);
        setResult(new TextResult(r->plain()));
        qDebug()<<"error rendering latex: "<<renderer->errorMessage();
    }

    delete result;

    renderer->deleteLater();
}

//saving code
QDomElement Expression::toXml(QDomDocument& doc)
{
    QDomElement expr=doc.createElement( QLatin1String("Expression") );
    QDomElement cmd=doc.createElement( QLatin1String("Command") );
    QDomText cmdText=doc.createTextNode( command() );
    cmd.appendChild( cmdText );
    expr.appendChild( cmd );
    if ( result() )
    {
        qDebug()<<"result: "<<result();
        QDomElement resXml=result()->toXml( doc );
        expr.appendChild( resXml );
    }

    return expr;
}

void Expression::saveAdditionalData(KZip* archive)
{
    //just pass this call to the result
    if(result())
        result()->saveAdditionalData(archive);
}

void Expression::addInformation(const QString& information)
{
    d->information.append(information);
}

QString Expression::additionalLatexHeaders()
{
    return QString();
}

int Expression::id()
{
    return d->id;
}

void Expression::setId(int id)
{
    d->id=id;
    emit idChanged();
}

void Expression::setFinishingBehavior(Expression::FinishingBehavior behavior)
{
    d->finishingBehavior=behavior;
}

Expression::FinishingBehavior Expression::finishingBehavior()
{
    return d->finishingBehavior;
}

void Expression::setInternal(bool internal)
{
    d->isInternal=internal;
}

bool Expression::isInternal()
{
    return d->isInternal;
}



