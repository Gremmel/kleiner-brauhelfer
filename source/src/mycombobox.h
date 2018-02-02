#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include <QComboBox>

class MyComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit MyComboBox(QWidget *parent = 0);

		void wheelEvent ( QWheelEvent * e );
signals:

public slots:

};

#endif // MYCOMBOBOX_H
