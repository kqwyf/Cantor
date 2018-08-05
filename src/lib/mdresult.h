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

#ifndef _MDRESULT_H
#define _MDRESULT_H

#include "epsresult.h"
#include "cantor_export.h"

namespace Cantor{
class MdResultPrivate;

class CANTOR_EXPORT MdResult : public EpsResult
{
  public:
    enum {Type=8};
    MdResult( const QString& code, const QUrl& url, const QString& plain = QString());
    MdResult( const QString& code, const QString& plain = QString());
    ~MdResult() override;
    
    QString toLatex() = 0;
    QVariant data() = 0;

    QDomElement toXml(QDomDocument& doc) = 0;
    
    void save(const QString& filename);

  private:
    MdResultPrivate* d;
};

}

#endif /* _MDRESULT_H */