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

#ifndef _SAGEEXPRESSION_H
#define _SAGEEXPRESSION_H

#include "expression.h"

class SageExpression : public MathematiK::Expression
{
  Q_OBJECT
  public:
    SageExpression( MathematiK::Session* session);
    ~SageExpression();

    void evaluate();
    void interrupt();

    void parseOutput(const QString& text);
    void parseError(const QString& text);

    void addFileResult(const QString& path);

    void onProcessError(const QString& msg);
  public slots:
    void evalFinished();

  protected:
    QString additionalLatexHeaders();
  private:
    QString m_outputCache;
    QString m_imagePath;
    bool m_isHelpRequest; 
    bool m_isContextHelpRequest;
    int m_promptCount;
};

#endif /* _SAGEEXPRESSION_H */
