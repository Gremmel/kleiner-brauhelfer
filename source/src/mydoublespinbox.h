#ifndef MYDOUBLESPINBOX_H
#define MYDOUBLESPINBOX_H

#include <QDoubleSpinBox>

class MyDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit MyDoubleSpinBox(QWidget *parent = 0);
		void wheelEvent ( QWheelEvent * e );
signals:

public slots:

};

#endif // MYDOUBLESPINBOX_H
