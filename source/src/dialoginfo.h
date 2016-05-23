#ifndef DIALOGINFO_H
#define DIALOGINFO_H

#include <QDialog>

namespace Ui {
  class DialogInfo;
}

class DialogInfo : public QDialog
{
  Q_OBJECT

public:
  static void Info(QWidget *parent = 0, QString title = "", QString infotext = "", bool isHtml = true);

private:
  explicit DialogInfo(QWidget *parent = 0, QString title = "", QString infotext = "", bool isHtml = true);
  ~DialogInfo();

private:
  Ui::DialogInfo *ui;
};

#endif // DIALOGINFO_H
