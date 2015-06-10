/****************************************************************************
**
** Copyright (C) 2006 Trolltech AS. All rights reserved.
**
** This file is part of the documentation of Qt. It was originally
** published as part of Qt Quarterly.
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation or under the
** terms of the Qt Commercial License Agreement. The respective license
** texts for these are provided with the open source and commercial
** editions of Qt.
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "faderwidget.h"

FaderWidget::FaderWidget(QWidget *parent)
    : QWidget(parent)
{
	if (parent)
		startBrush = parent->palette().window();
	else
		startBrush = Qt::white;

	timeLine = new QTimeLine(250, this);
	timeLine->setFrameRange(0, 1000);
	connect(timeLine, SIGNAL(frameChanged(int)), this, SLOT(update()));
	connect(timeLine, SIGNAL(finished()),this, SIGNAL(sig_fertig()));

	setAttribute(Qt::WA_DeleteOnClose);
	resize(parent->size());
}

void FaderWidget::start()
{
	timeLine->start();
	show();
}

void FaderWidget::setFadeColor(QColor color)
{
	startBrush = color;
}

void FaderWidget::paintEvent(QPaintEvent * /* event */)
{
	QPainter painter(this);
	qreal frame = timeLine->currentFrame();
	painter.setOpacity(frame / 1000.);
	painter.fillRect(rect(), startBrush);

	if (frame >= 1000)
		close();
}
