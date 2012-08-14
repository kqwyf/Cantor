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
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#include "commandentry.h"
#include "animationresultitem.h"
#include "lib/result.h"
#include "lib/animationresult.h"

#include <KFileDialog>
#include <KDebug>

AnimationResultItem::AnimationResultItem(QGraphicsObject* parent)
    : WorksheetImageItem(parent), ResultItem(), m_height(0), m_movie(0)
{
    connect(this, SIGNAL(removeResult()), parentEntry(),
	    SLOT(removeResult()));
}

AnimationResultItem::~AnimationResultItem()
{
}

double AnimationResultItem::setGeometry(double x, double y, double w)
{
    Q_UNUSED(w);
    setPos(x,y);

    return m_height;
}

void AnimationResultItem::populateMenu(KMenu* menu, const QPointF& pos)
{
    addCommonActions(this, menu);

    menu->addSeparator();
    kDebug() << "populate Menu";
    emit menuCreated(menu, mapToParent(pos));
}

ResultItem* AnimationResultItem::updateFromResult(Cantor::Result* result)
{
    QMovie* mov;
    switch(result->type()) {
    case Cantor::AnimationResult::Type:
	mov = static_cast<QMovie*>(result->data().value<QObject*>());
	setMovie(mov);
	return this;
    default:
	deleteLater();
	return create(parentEntry(), result);
    }
}

QRectF AnimationResultItem::boundingRect() const
{
    return QRectF(0, 0, width(), height());
}

void AnimationResultItem::setMovie(QMovie* movie)
{
    if (m_movie) {
	m_movie->disconnect(this, SLOT(updateFrame()));
	m_movie->disconnect(this, SLOT(updateSize()));
    }
    m_movie = movie;
    m_height = 0;
    connect(m_movie, SIGNAL(frameChanged(int)), this,
	    SLOT(updateFrame()));
    connect(m_movie, SIGNAL(resized(const QSize&)), 
	    this, SLOT(updateSize(const QSize&)));
}

void AnimationResultItem::updateFrame()
{
    setImage(m_movie->currentImage());
}

void AnimationResultItem::updateSize(const QSize& size)
{
    if (m_height != size.height()) {
	m_height = size.height();
	emit sizeChanged();
    }
}

void AnimationResultItem::saveResult()
{
    Cantor::Result* res = result();
    const QString& filename=KFileDialog::getSaveFileName(KUrl(), res->mimeType(), worksheet()->worksheetView());
    kDebug()<<"saving result to "<<filename;
    res->save(filename);
}

void AnimationResultItem::deleteLater()
{
    WorksheetImageItem::deleteLater();
}

CommandEntry* AnimationResultItem::parentEntry()
{
    return qobject_cast<CommandEntry*>(parentObject());
}

Cantor::Result* AnimationResultItem::result()
{
    return parentEntry()->expression()->result();
}

#include "animationresultitem.moc"