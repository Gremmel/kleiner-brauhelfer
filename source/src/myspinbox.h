#ifndef MYSPINBOX_H
#define MYSPINBOX_H

#include <QSpinBox>

class MySpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit MySpinBox(QWidget *parent = 0);
		void wheelEvent ( QWheelEvent * e );
signals:

public slots:

};

#endif // MYSPINBOX_H
