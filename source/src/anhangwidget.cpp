#include "anhangwidget.h"
#include "ui_anhangwidget.h"
#include <QFileDialog>

bool AnhangWidget::isImage(const QString pfad)
{
  QFileInfo fileInfo(pfad);
  QString ext = fileInfo.suffix();
  return (ext == "png" || ext == "svg" || ext == "gif" || ext == "jpg" || ext == "jpeg" || ext == "bmp");
}

AnhangWidget::AnhangWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::AnhangWidget)
{
  ui->setupUi(this);
  ui->bild->setVisible(false);
}

AnhangWidget::~AnhangWidget()
{
  delete ui;
}

void AnhangWidget::setID(int value)
{
  ID = value;
}

int AnhangWidget::getID() const
{
  return ID;
}

QString AnhangWidget::getPfad() const
{
  return ui->lineEdit_Pfad->text();
}

QString AnhangWidget::getFullPfad() const
{
  if (QDir::isRelativePath(ui->lineEdit_Pfad->text()))
    return dir.filePath(ui->lineEdit_Pfad->text());
  else
    return ui->lineEdit_Pfad->text();
}

void AnhangWidget::setPfad(const QString pfad)
{
  ui->lineEdit_Pfad->setText(pfad);
  if (isImage(pfad))
  {
    QPixmap pix(getFullPfad());
    ui->bild->setPixmap(pix.scaled(ui->bild->width(), ui->bild->height(), Qt::KeepAspectRatio));
    ui->checkBox_Relativ->setChecked(QDir::isRelativePath(pfad));
    ui->bild->setVisible(true);
  }
  else
  {
    ui->bild->setVisible(false);
  }
}

void AnhangWidget::setBasisPfad(const QString basis)
{
  dir.setPath(basis);
}

void AnhangWidget::openDialog()
{
  QString fileName = QFileDialog::getOpenFileName(this, trUtf8("Anhang auswählen"), getFullPfad());
  if (!fileName.isEmpty())
  {
    if (ui->checkBox_Relativ->isChecked())
      setPfad(dir.relativeFilePath(fileName));
    else
      setPfad(fileName);
    emit sig_Aenderung();
  }
}

void AnhangWidget::closeEvent(QCloseEvent *)
{
  emit sig_vorClose(ID);
}

void AnhangWidget::on_pushButton_close_clicked()
{
  close();
}

void AnhangWidget::on_lineEdit_editingFinished()
{
  emit sig_Aenderung();
}

void AnhangWidget::on_pushButton_Browse_clicked()
{
  openDialog();
}

void AnhangWidget::on_checkBox_Relativ_clicked()
{
  QString pfad = getPfad();
  if (ui->checkBox_Relativ->isChecked())
  {
    if (QDir::isRelativePath(pfad))
      setPfad(pfad);
    else
      setPfad(dir.relativeFilePath(pfad));
  }
  else
  {
    setPfad(getFullPfad());
  }
  emit sig_Aenderung();
}
