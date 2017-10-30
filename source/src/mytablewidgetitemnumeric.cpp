#include "mytablewidgetitemnumeric.h"

bool MyTableWidgetItemNumeric::operator<(const QTableWidgetItem &other) const
{
    return QLocale::system().toDouble(text()) < QLocale::system().toDouble(other.text());
}
