#ifndef DIALOGDATUM_H
#define DIALOGDATUM_H

#include <QDialog>
#include <QDate>

namespace Ui {
class DialogDatum;
}

class DialogDatum : public QDialog
{
  Q_OBJECT

public:
  explicit DialogDatum(QWidget *parent = 0, QString title = "", QString buttontext = "");
  ~DialogDatum();
  QDate getDatum();

private slots:
  void on_pushButton_clicked();

private:
  Ui::DialogDatum *ui;
};

#endif // DIALOGDATUM_H
