#include "doubleeditlineimpl.h"
#include "definitionen.h"

doubleEditLineImpl::doubleEditLineImpl( QWidget * parent, Qt::WindowFlags f) 
	: QWidget(parent, f)
{
	setupUi(this);
	icon_achtung->setVisible(false);
	icon_warnung->setVisible(false);
	icon_info->setVisible(false);
}

void doubleEditLineImpl::setRest(double rest)
{
    QPalette palette = spinBox_Wert -> palette();
    if (rest < 0.0)
        palette.setColor(QPalette::WindowText, QColor::fromRgb(FARBE_COMBO_ROHSTOFF_EMPTY_HELL));
    label_Rest->setPalette(palette);
    label_Rest->setText(QLocale().toString(rest, 'f', spinBox_Wert->decimals()));
}

void doubleEditLineImpl::setRestVisible(bool visible)
{
    widget_Rest->setVisible(visible);
}
