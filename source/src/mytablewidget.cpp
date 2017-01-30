#include "mytablewidget.h"
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>

MyTableWidget::MyTableWidget(QWidget *parent) :
    QTableWidget(parent)
{
}

void MyTableWidget::keyPressEvent(QKeyEvent *event)
{
    // Copy selection to clipboard
    if (event->key() == Qt::Key_C && (event->modifiers() & Qt::ControlModifier))
    {
        QModelIndexList cells = selectedIndexes();
        qSort(cells); // Necessary, otherwise they are in column order

        QString text;
        int currentRow = 0; // To determine when to insert newlines
        foreach (const QModelIndex& cell, cells) {
            if (text.length() == 0) {
                // First item
            } else if (cell.row() != currentRow) {
                // New row
                text += '\n';
            } else {
                // Next cell
                text += '\t';
            }
            currentRow = cell.row();
            text += cell.data().toString();
        }
        QApplication::clipboard()->setText(text);
    }
    else
    {
        return QTableWidget::keyPressEvent(event);
    }
}
