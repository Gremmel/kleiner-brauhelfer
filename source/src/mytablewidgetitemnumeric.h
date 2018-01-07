#ifndef MYTABLEWIDGETITEMNUMERIC_H
#define MYTABLEWIDGETITEMNUMERIC_H

#include <QTableWidgetItem>

class MyTableWidgetItemNumeric : public QTableWidgetItem
{
public:
    MyTableWidgetItemNumeric(double value = 0.0, int type = Type);
    MyTableWidgetItemNumeric(const QIcon &icon, double value = 0.0, int type = Type);
    bool operator <(const QTableWidgetItem &other) const;
    double value() const;
    void setValue(double value);
private:
    double mValue;
};

#endif // MYTABLEWIDGETITEMNUMERIC_H
