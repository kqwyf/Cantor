#include "markdown.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
using std::cout;
using std::endl;
using std::cerr;

QString MdResult::toHtml()
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
