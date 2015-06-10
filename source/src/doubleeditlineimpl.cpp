#include "doubleeditlineimpl.h"
//
doubleEditLineImpl::doubleEditLineImpl( QWidget * parent, Qt::WindowFlags f) 
	: QWidget(parent, f)
{
	setupUi(this);
	icon_achtung->setVisible(false);
	icon_warnung->setVisible(false);
	icon_info->setVisible(false);
}
//

