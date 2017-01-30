#ifndef MYTABLEWIDGET_H
#define MYTABLEWIDGET_H

#include <QTableWidget>

class MyTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit MyTableWidget(QWidget *parent = Q_NULLPTR);
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
};

#endif // MYTABLEWIDGET_H
