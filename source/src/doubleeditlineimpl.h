#ifndef DOUBLEEDITLINEIMPL_H
#define DOUBLEEDITLINEIMPL_H
//
#include <QWidget>
#include "ui_doubleEditLine.h"
//
class doubleEditLineImpl : public QWidget, public Ui::Form
{
Q_OBJECT
public:
    doubleEditLineImpl( QWidget * parent = 0,  Qt::WindowFlags f = 0 );
    void setRest(double rest);
    void setRestVisible(bool visible);
};
#endif





