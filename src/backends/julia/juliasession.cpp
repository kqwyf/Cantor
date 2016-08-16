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
    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#include "juliasession.h"

#include <KProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QStandardPaths>

#include "juliaexpression.h"
#include "settings.h"

JuliaSession::JuliaSession(Cantor::Backend *backend)
    : Session(backend)
    , m_process(nullptr)
    , m_interface(nullptr)
    , m_currentExpression(nullptr)
{
}

void JuliaSession::login()
{
    if (m_process) {
        m_process->deleteLater();
    }

    m_process = new KProcess(this);
    m_process->setOutputChannelMode(KProcess::SeparateChannels);

    (*m_process)
        << QStandardPaths::findExecutable(QLatin1String("cantor_juliaserver"));

    m_process->start();

    m_process->waitForStarted();
    m_process->waitForReadyRead();
    QTextStream stream(m_process->readAllStandardOutput());

    QString readyStatus = QLatin1String("ready");
    while (m_process->state() == QProcess::Running) {
        const QString &rl = stream.readLine();
        if (rl == readyStatus) {
            break;
        }
    }

    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning() << "Can't connect to the D-Bus session bus.\n"
                      "To start it, run: eval `dbus-launch --auto-syntax`";
        return;
    }

    const QString &serviceName =
        QString::fromLatin1("org.kde.Cantor.Julia-%1").arg(m_process->pid());

    m_interface = new QDBusInterface(
        serviceName,
        QString::fromLatin1("/"),
        QString(),
        QDBusConnection::sessionBus()
    );

    if (not m_interface->isValid()) {
        qWarning() << QDBusConnection::sessionBus().lastError().message();
        return;
    }

    m_interface->call(
        QString::fromLatin1("login"),
        JuliaSettings::self()->replPath().path()
    );

    emit ready();
}

void JuliaSession::logout()
{
    m_process->terminate();
    changeStatus(Cantor::Session::Done);
}

void JuliaSession::interrupt()
{
    if (m_process->pid()) {
        m_process->kill();
    }

    for (Cantor::Expression *e : m_runningExpressions) {
        e->interrupt();
    }

    m_runningExpressions.clear();
    changeStatus(Cantor::Session::Done);
}

Cantor::Expression *JuliaSession::evaluateExpression(
    const QString &cmd,
    Cantor::Expression::FinishingBehavior behave)
{
    JuliaExpression *expr = new JuliaExpression(this);

    changeStatus(Cantor::Session::Running);

    expr->setFinishingBehavior(behave);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

Cantor::CompletionObject *JuliaSession::completionFor(
    const QString &command,
    int index)
{
    Q_UNUSED(command);
    Q_UNUSED(index);
    return nullptr;
}

void JuliaSession::runJuliaCommand(const QString &command) const
{
    m_interface->call(QLatin1String("runJuliaCommand"), command);
}

void JuliaSession::runJuliaCommandAsync(const QString &command)
{
    m_interface->callWithCallback(
        QLatin1String("runJuliaCommand"),
        {command},
        this,
        SLOT(onResultReady())
    );
}

void JuliaSession::onResultReady()
{
    m_currentExpression->finalize();
    m_runningExpressions.removeAll(m_currentExpression);

    changeStatus(Cantor::Session::Done);
}

void JuliaSession::runExpression(JuliaExpression *expr)
{
    m_runningExpressions.append(expr);
    m_currentExpression = expr;
    runJuliaCommandAsync(expr->command());
}

QString JuliaSession::getStringFromServer(const QString &method)
{
    const QDBusReply<QString> &reply = m_interface->call(method);
    return (reply.isValid() ? reply.value() : reply.error().message());
}

QString JuliaSession::getOutput()
{
    return getStringFromServer(QLatin1String("getOutput"));
}

QString JuliaSession::getError()
{
    return getStringFromServer(QLatin1String("getError"));
}

bool JuliaSession::getWasException()
{
    const QDBusReply<bool> &reply =
        m_interface->call(QLatin1String("getWasException"));
    return reply.isValid() and reply.value();
}

#include "juliasession.moc"