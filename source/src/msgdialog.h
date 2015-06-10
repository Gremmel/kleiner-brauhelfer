#ifndef MSGDIALOG_H
#define MSGDIALOG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

namespace Ui {
class MsgDialog;
}

class MsgDialog : public QDialog
{
	Q_OBJECT

signals:
	void sig_checkFertig(int count);	
	
public:
	explicit MsgDialog(QWidget *parent = 0);
	~MsgDialog();
	void checkMsgAnzeigen(QList<int> idList);
	void setURL(QString value);
	QList<int> getIgnorIDList();
	
private slots:
  void httpFinished();
	void httpReadyRead();
	
	void on_button_next_clicked();
	
private:
	void ladeMessage();
	void checkFilter();
	void MeldungEintragen(int nr);

	Ui::MsgDialog *ui;
	bool msgAnzeigen;
	//aktuell Angezeigte Message Nummer (forlaufen beginnt bei 0)
	int aktMsg;
	//akteulle ID der Angezeigten Msg 
	int aktID;
	QUrl url;
	QList<int> IgnorIDListe;
	QNetworkAccessManager qnam;
	QNetworkReply *reply;
	QString rohMeldungen;
	QStringList Meldungen;
};

#endif // MSGDIALOG_H
