#include "markdown.h"
#include <isotream>
#include <sstream>
#include <fstream>
#include <vector>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <QRegExp>
#include <QUrl>
using std::cout;
using std::endl;
using std::cerr;

class Cantor::MdResultPrivate
{
  public:
    MdResultPrivate()
    {
        hasUrl = false;
    }

    bool hasUrl;
    QString code;
    QString plain;
};

MdResult::MdResult(const QString& code, const QString& plain) : d(new LatexResultPrivate)
{
    d->code=code;
    d->plain=plain;
}

LatexResult::LatexResult(const QString& code, const QUrl &url, const QString& plain) : EpsResult( url ),
                                                                                       d(new LatexResultPrivate)
{
    d->code=code;
    d->hasUrl=true;
    d->plain=plain;
}

MdResult::~MdResult()
{
    delete d;
}

// same in result.cpp
QString MdResult::toLatex()
{
    QString html=toHtml();
    //replace linebreaks
    html.replace(QRegExp(QLatin1String("<br/>[\n]")), QLatin1String("\n"));
    //remove all the unknown tags
    html.remove( QRegExp( QLatin1String("<[a-zA-Z\\/][^>]*>") ) );
    return QString::fromLatin1("\\begin{verbatim} %1 \\end{verbatim}").arg(html);
}

QVariant MdResult::data()
{
    return QVariant(d->code);
}

QDomElement MdResult::toXml(QDomDocument& doc)
{
    qDebug()<<"saving textresult "<<toHtml();
    QDomElement e=doc.createElement(QLatin1String("Result"));
    e.setAttribute(QLatin1String("type"), QLatin1String("application/x-markdown"));
    if(d->hasUrl == true){
        QUrl url= EpsResult::data().toUrl();
        e.setAttribute(QLatin1String("filename"), url.fileName());
    }
    QDomText txt=doc.createTextNode(data().toString());
    e.appendChild(txt);

    return e;  
}

// not implemented in LatexResult.cpp
void MdResult::saveAdditionalData(KZip* archive)
{
    if(d->hasUrl == true)
    {
        EpsResult::saveAdditionalData(archive);
    }
}

void MdResult::save(const QString& filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);

    stream<<code();

    file.close();
}