#include "markdown.h"
#include <iostream>
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

class Cantor::MarkdownResultPrivate
{
  public:
    MarkdownResultPrivate()
    {
        hasUrl = false;
    }

    bool hasUrl;
    QString code;
    QString plain;
};

MarkdownResult::MarkdownResult(const QString& code) : d(new MarkdownResultPrivate)
{
    d->code=code.trimmed();
    d->plain=code.trimmed();
}

MarkdownResult::MarkdownResult(const QString& code, const QString& plain) : d(new MarkdownResultPrivate)
{
    d->code=code;
    d->plain=plain;
}

MarkdownResult::~MarkdownResult()
{
    delete d;
}

QString MarkdownResult::toHtml()
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
    return QString(QString::fromLocal8Bit(ans.c_str()));
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

QVariant MarkdownResult::data()
{
    return QVariant(d->code);
}

QDomElement MarkdownResult::toXml(QDomDocument& doc)
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
void MarkdownResult::saveAdditionalData(KZip* archive)
{
    if(d->hasUrl == true)
    {
        EpsResult::saveAdditionalData(archive);
    }
}

void MarkdownResult::save(const QString& filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);

    stream<<code();

    file.close();
}
