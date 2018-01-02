#ifndef ANHANGWIDGET_H
#define ANHANGWIDGET_H

#include <QWidget>
#include <QDir>

namespace Ui {
  class AnhangWidget;
}

class AnhangWidget : public QWidget
{
  Q_OBJECT

public:
  static bool isImage(const QString pfad);

  explicit AnhangWidget(QWidget *parent = 0);
  ~AnhangWidget();
  void setID(int value);
  int getID() const;
  QString getPfad() const;
  QString getFullPfad() const;
  void setPfad(const QString pfad);
  void setBasisPfad(const QString basis);
  void openDialog();

signals:
	void sig_vorClose(int ID);
	void sig_Aenderung();

private slots:
	void on_pushButton_close_clicked();
    void on_lineEdit_Pfad_editingFinished();
	void on_pushButton_Browse_clicked();
	void on_checkBox_Relativ_clicked();

protected:
	void closeEvent(QCloseEvent *event);

private:
  Ui::AnhangWidget *ui;
  int ID;
  QDir dir;
};

#endif // ANHANGWIDGET_H
