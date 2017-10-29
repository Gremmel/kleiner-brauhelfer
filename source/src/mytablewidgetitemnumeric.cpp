#include "mytablewidgetitemnumeric.h"

bool MyTableWidgetItemNumeric::operator<(const QTableWidgetItem &other) const
{
    return text().toDouble() < other.text().toDouble();
}
