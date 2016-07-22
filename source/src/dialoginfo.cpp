#include "dialoginfo.h"
#include "ui_dialoginfo.h"

DialogInfo::DialogInfo(QWidget *parent, QString title, QString infotext, bool isHtml) :
  QDialog(parent),
  ui(new Ui::DialogInfo)
{
  ui->setupUi(this);
  setWindowTitle(title);
  if (isHtml)
    ui->textInfo->setHtml(infotext);
  else
    ui->textInfo->setText(infotext);
}

DialogInfo::~DialogInfo()
{
  delete ui;
}

void DialogInfo::Info(QWidget *parent, QString title, QString infotext, bool isHtml)
{
  DialogInfo* dlg = new DialogInfo(parent, title, infotext, isHtml);
  dlg->setWindowFlags(dlg->windowFlags()^Qt::WindowContextHelpButtonHint);
  dlg->exec();
  delete dlg;
}
