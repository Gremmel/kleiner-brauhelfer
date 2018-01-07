#include "mytablewidgetitemnumeric.h"

MyTableWidgetItemNumeric::MyTableWidgetItemNumeric(double value, int type) :
    QTableWidgetItem(type)
{
    setValue(value);
}

MyTableWidgetItemNumeric::MyTableWidgetItemNumeric(const QIcon &icon, double value, int type) :
    QTableWidgetItem(icon, "", type)
{
    setValue(value);
}

bool MyTableWidgetItemNumeric::operator<(const QTableWidgetItem &other) const
{
    const MyTableWidgetItemNumeric* ptr = static_cast<const MyTableWidgetItemNumeric*>(&other);
    if (ptr)
        return value() < ptr->value();
    return false;
}

double MyTableWidgetItemNumeric::value() const
{
    return mValue;
}

void MyTableWidgetItemNumeric::setValue(double value)
{
    mValue = value;
    setText(QLocale().toString(mValue));
}
