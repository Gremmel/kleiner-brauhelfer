#include "rastwidget.h"
#include "ui_rastwidget.h"
#include "definitionen.h"

Rastwidget::Rastwidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Rastwidget)
{
	ui->setupUi(this);
	ui->comboBox_Rast->addItem(trUtf8("Gummirast (35°-40°)"));
	ui->comboBox_Rast->addItem(trUtf8("Weizenrast (45°)"));
	ui->comboBox_Rast->addItem(trUtf8("Eiweißrast (57°)"));
	ui->comboBox_Rast->addItem(trUtf8("Maltoserast (60°-65°)"));
	ui->comboBox_Rast->addItem(trUtf8("Kombirast (66°-69°)"));
	ui->comboBox_Rast->addItem(trUtf8("Verzuckerung (70°-75°)"));
	ui->comboBox_Rast->addItem(trUtf8("Abmaischen (78°)"));
	ui->comboBox_Rast->lineEdit()->setText("");
	ui->comboBox_Rast->setCurrentIndex(-1);
	ui->spinBox_RastDauer->setValue(0);
	ui->spinBox_RastTemp->setValue(0);
	//setVisible(false);
	animationAktiv = false;

}

Rastwidget::~Rastwidget()
{
	delete ui;
}

void Rastwidget::setRastName(QString Name)
{
	ui->comboBox_Rast -> lineEdit() -> setText(Name);

}

QString Rastwidget::getRastName()
{
	return ui->comboBox_Rast->currentText();
}

void Rastwidget::setRastTemp(int Value)
{
	ui->spinBox_RastTemp->setValue(Value);
}

int Rastwidget::getRastTemp()
{
	return ui->spinBox_RastTemp->value();
}

void Rastwidget::setRastDauer(int Value)
{
	ui->spinBox_RastDauer->setValue(Value);
}

int Rastwidget::getRastDauer()
{
	return ui->spinBox_RastDauer->value();
}

void Rastwidget::setID(int Value)
{
	id = Value;
}

void Rastwidget::setAnimationAus(bool value)
{
	animationAus = value;
}

int Rastwidget::getID()
{
	return id;
}

void Rastwidget::setDisabled(bool status)
{
	QAbstractSpinBox::ButtonSymbols bs;

	if (!status) {
		bs = QAbstractSpinBox::UpDownArrows;
	}
	else {
		bs = QAbstractSpinBox::NoButtons;
	}
	ui->spinBox_RastDauer->setButtonSymbols(bs);
	ui->spinBox_RastTemp->setButtonSymbols(bs);
	ui->spinBox_RastDauer->setReadOnly(status);
	ui->spinBox_RastTemp->setReadOnly(status);
	ui->comboBox_Rast->setDisabled(status);
	ui->pushButton_close->setVisible(!status);
	ui->pushButton_NachOben->setVisible(!status);
	ui->pushButton_NachUnten->setVisible(!status);
}

void Rastwidget::on_close()
{
	close();
}

void Rastwidget::slot_animation_fertig()
{
	animationAktiv = false;
}

void Rastwidget::closeEvent(QCloseEvent *)
{
	emit sig_vorClose(id);
}

void Rastwidget::moveEvent(QMoveEvent *event)
{
	if (!animationAktiv){
		QPoint p;
		//Widget zur neuen Position animieren
		animation = new QPropertyAnimation(this, "pos");
		animation->setDuration(250);
		if ((event->oldPos().x() == event->pos().x()) && ((event->oldPos().y() == event->pos().y()))){
			p.setX(event->oldPos().x());
			p.setY(0);
			animation->setStartValue(p);
		}
		else {
			animation->setStartValue(event->oldPos());
		}
		animation->setEndValue(event->pos());
        connect(animation, SIGNAL(finished()), this, SLOT(slot_animation_fertig()));
		if (!animationAus){
			animationAktiv = true;
            animation->start(QAbstractAnimation::DeleteWhenStopped);
		}
	}

}


void Rastwidget::on_pushButton_NachOben_clicked()
{
	emit sig_aenderung(id);
	emit sig_nachOben(id);
}

void Rastwidget::on_pushButton_NachUnten_clicked()
{
	emit sig_aenderung(id);
	emit sig_nachUnten(id);
}

void Rastwidget::on_pushButton_close_clicked()
{
	//Widget seitlich rausschieben
	animation = new QPropertyAnimation(this, "pos");
	animation->setDuration(250);
	animation->setStartValue(this->pos());
	animation->setEndValue(QPoint(-250,this->pos().y()));
	connect(animation, SIGNAL(finished()), this, SLOT(on_close()));
    connect(animation, SIGNAL(finished()), this, SLOT(slot_animation_fertig()));
	animationAktiv = true;
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}


void Rastwidget::on_comboBox_Rast_currentIndexChanged(int index)
{
	//Beim wechseln Defaulttemperatur eintragen
	//Gummirast
	if (index == 0){
		ui->spinBox_RastTemp->setValue(38);
		ui->spinBox_RastDauer->setValue(60);
	}
	//Weizenrast
	else if (index == 1){
		ui->spinBox_RastTemp->setValue(45);
		ui->spinBox_RastDauer->setValue(15);
	}
	//Eiweißrast
	else if (index == 2){
		ui->spinBox_RastTemp->setValue(57);
		ui->spinBox_RastDauer->setValue(10);
	}
	//Maltoserast
	else if (index == 3){
		ui->spinBox_RastTemp->setValue(63);
		ui->spinBox_RastDauer->setValue(35);
	}
	//Kombirast
	else if (index == 4){
		ui->spinBox_RastTemp->setValue(67);
		ui->spinBox_RastDauer->setValue(60);
	}
	//Verzuckerung
	else if (index == 5){
		ui->spinBox_RastTemp->setValue(72);
		ui->spinBox_RastDauer->setValue(20);
	}
	//Abmaischen
	else if (index == 6){
		ui->spinBox_RastTemp->setValue(78);
		ui->spinBox_RastDauer->setValue(0);
	}
	emit sig_aenderung(id);
}

void Rastwidget::on_spinBox_RastTemp_valueChanged(int )
{
	emit sig_aenderung(id);
}

void Rastwidget::on_spinBox_RastDauer_valueChanged(int)
{
	emit sig_aenderung(id);
}
