/*
Copyright (c) 2006-2007, Tom Thielicke IT Solutions

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.
*/

/****************************************************************
**
** Definition of the ErrorMessage class
** File name: errormessage.h
**
****************************************************************/

#ifndef ERRORMESSAGE_H
#define ERRORMESSAGE_H

#include <QWidget>
#include <QString>

//! The ErrorMessage class provides an error message.
/*!
	The ErrorMessage class puts different parameters together to a full error
	message.

	@author Tom Thielicke, s712715
	@version 0.0.3
	@date 02.07.2006
*/
class ErrorMessage : public QWidget {
	Q_OBJECT

	public:

		//! Empty constructor.
		/*!
			@param parent The parent QWidget
		*/
		ErrorMessage(QWidget *parent = 0);

		//! Shows a message window
		/*!
			This function puts different parameters together using the
			functions getCancelText() and getErrorText. After that ist shows
			the message on the screen.

			@param errorNo Number of the error
			@param errorType Number of the message type
			@param cancelProcedure Number of an additional cancel text
			@param addon Free text to apend it on the message text
			@see getCancelText(), getErrorText()
		*/
		void showMessage(int errorNo, int errorType, int cancelProcedure,
			QString addon = "");

	private:

		//! Selects the corresponding cancel text of the cancel text number
		/*!
			@param number Number of the cancel text
			@return Corresponding cancel text
		*/
		QString getCancelText(int number);

		//! Selects the corresponding error text of the error text number
		/*!
			@param number Number of the error text
			@return Corresponding error text
		*/
		QString getErrorText(int number);
};

#endif //ERRORMESSAGE_H
