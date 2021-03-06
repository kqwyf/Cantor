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
    Copyright (C) 2012 Filipe Saraiva <filipe@kde.org>
 */

#ifndef _PYTHONBACKEND_H
#define _PYTHONBACKEND_H

#include "backend.h"

class CANTOR_EXPORT PythonBackend : public Cantor::Backend
{
  Q_OBJECT
  public:
    explicit PythonBackend(QObject* parent = nullptr, const QList<QVariant> args = QList<QVariant>());
    ~PythonBackend() override;

    Cantor::Backend::Capabilities capabilities() const Q_DECL_OVERRIDE;

    QWidget* settingsWidget(QWidget* parent) const Q_DECL_OVERRIDE;
    KConfigSkeleton* config() const Q_DECL_OVERRIDE = 0;
};


#endif /* _PYTHONBACKEND_H */
