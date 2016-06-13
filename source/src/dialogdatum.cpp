#include "dialogdatum.h"
#include "ui_dialogdatum.h"

DialogDatum::DialogDatum(QWidget *parent, QString title, QString buttontext) :
  QDialog(parent),
  ui(new Ui::DialogDatum)
{
  ui->setupUi(this);
  setWindowTitle(title);
  ui->pushButton->setText(buttontext);
}

DialogDatum::~DialogDatum()
{
  delete ui;
}

QDate DialogDatum::getDatum()
{
  return ui->calendarWidget->selectedDate();
}

void DialogDatum::on_pushButton_clicked()
{
  close();
}
