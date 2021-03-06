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

#ifndef _SAGECOMPLETIONOBJECT_H
#define _SAGECOMPLETIONOBJECT_H

#include "completionobject.h"

class SageSession;

namespace Cantor{
    class Expression;
}

class SageCompletionObject : public Cantor::CompletionObject
{
  Q_OBJECT
  public:
    SageCompletionObject( const QString& command, int index, SageSession* session);
    ~SageCompletionObject() override;

  protected:
    bool mayIdentifierContain(QChar c) const Q_DECL_OVERRIDE;
    bool mayIdentifierBeginWith(QChar c) const Q_DECL_OVERRIDE;


  protected Q_SLOTS:
    void fetchCompletions() Q_DECL_OVERRIDE;
    void extractCompletions();
    void fetchIdentifierType() Q_DECL_OVERRIDE;
    void extractIdentifierType();

 private:
    void extractCompletionsNew();
    void extractCompletionsLegacy();

  private:
    Cantor::Expression* m_expression;
};

#endif /* _SAGECOMPLETIONOBJECT_H */
