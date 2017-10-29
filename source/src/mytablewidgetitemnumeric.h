#ifndef MYTABLEWIDGETITEMNUMERIC_H
#define MYTABLEWIDGETITEMNUMERIC_H

#include <QTableWidgetItem>

class MyTableWidgetItemNumeric : public QTableWidgetItem
{
public:
    MyTableWidgetItemNumeric(int type = Type) : QTableWidgetItem(type) {}
    MyTableWidgetItemNumeric(const QString &text, int type = Type) : QTableWidgetItem(text, type) {}
    MyTableWidgetItemNumeric(const QIcon &icon, const QString &text, int type = Type) : QTableWidgetItem(icon, text, type) {}
    bool operator <(const QTableWidgetItem &other) const;
};

#endif // MYTABLEWIDGETITEMNUMERIC_H
