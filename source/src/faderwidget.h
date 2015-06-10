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

#ifndef FADERWIDGET_H
#define FADERWIDGET_H

#include <QWidget>
#include <QTimeLine>


class FaderWidget : public QWidget
{
	Q_OBJECT
	Q_PROPERTY(QBrush fadeBrush READ fadeBrush WRITE setFadeBrush)
	Q_PROPERTY(int fadeDuration READ fadeDuration WRITE setFadeDuration)
public:

	FaderWidget(QWidget *parent);

	QBrush fadeBrush() const { return startBrush; }
	void setFadeBrush(const QBrush &newColor) { startBrush = newColor; }

	int fadeDuration() const { return timeLine->duration(); }
	void setFadeDuration(int milliseconds) { timeLine->setDuration(milliseconds); }

	void start();
	void setFadeColor(QColor color);

protected:
	void paintEvent(QPaintEvent *event);

private:
		QTimeLine *timeLine;
		QBrush startBrush;

signals:
		void sig_fertig();
};

#endif
