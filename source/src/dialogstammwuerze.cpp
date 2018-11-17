#include "dialogstammwuerze.h"
#include "ui_dialogstammwuerze.h"
#include <QSettings>
#include "definitionen.h"
#include "berechnungen.h"

DialogStammwuerze::DialogStammwuerze(double value, double sw, double temp, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogStammwuerze)
{
    ui->setupUi(this);
    LeseKonfig();
    SWAnstellen = sw;
    ui->spinBox_SwPlato->setValue(value);
    ui->spinBox_S_Temperatur->setValue(temp);
    if (ui->spinBox_S_Temperatur->value() == 20.0)
    {
        ui->spinBox_S_SwPlato->setValue(value);
        ui->spinBox_S_SwDichte->setValue(QBerechnungen::GetDichte(value));
    }
    else
    {
        ui->spinBox_S_SwPlato->setValue(0.0);
        ui->spinBox_S_SwDichte->setValue(0.0);
    }
    ui->spinBox_R_SwBrix->setValue(SWAnstellen == 0.0 ? value * ui->spinBox_R_Factor->value() : 0.0);
    ui->spinBox_SwPlato->setFocus();
}

DialogStammwuerze::~DialogStammwuerze()
{
    delete ui;
}

double DialogStammwuerze::value() const
{
    return ui->spinBox_SwPlato->value();
}

double DialogStammwuerze::temperature() const
{
    return ui->spinBox_S_Temperatur->value();
}

void DialogStammwuerze::LeseKonfig()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, KONFIG_ORDNER, APP_KONFIG);

    double d;
    settings.beginGroup("Erweitert");
    d = settings.value("FaktorBrixPlato").toDouble();

    //Default wert wenn Wert in Konfigdatei noch nicht vorhanden ist
    if (d == 0){
        d = 1.03;
    }
    ui->spinBox_R_Factor->setValue(d);

    //Formel zur Umrechnung von Brix nach Plato im Gärverlauf
    ui->comboBox_FormelBrixPlato->setCurrentText(settings.value("FormelBrixPlato").toString());

    //Gärungskorrektur
    d = settings.value("Gaerungskorrektur").toDouble();
    settings.endGroup();

    if (d == 0){
        d = 0.44552;
    }
    Gaerungskorrektur = d;
}

void DialogStammwuerze::on_spinBox_S_SwPlato_valueChanged(double value)
{
    if (ui->spinBox_S_SwPlato->hasFocus())
    {
        ui->spinBox_S_SwDichte->setValue(QBerechnungen::GetDichte(value));
        ui->spinBox_SwPlato->setValue(QBerechnungen::densityAtX(ui->spinBox_S_SwPlato->value(), ui->spinBox_S_Temperatur->value(), 20));
        ui->spinBox_R_SwBrix->setValue(SWAnstellen == 0.0 ? ui->spinBox_SwPlato->value() * ui->spinBox_R_Factor->value() : 0.0);
    }
}

void DialogStammwuerze::on_spinBox_S_SwDichte_valueChanged(double value)
{
    if (ui->spinBox_S_SwDichte->hasFocus())
    {
        ui->spinBox_S_SwPlato->setValue(QBerechnungen::GetGradPlato(value));
        ui->spinBox_SwPlato->setValue(QBerechnungen::densityAtX(ui->spinBox_S_SwPlato->value(), ui->spinBox_S_Temperatur->value(), 20));
        ui->spinBox_R_SwBrix->setValue(SWAnstellen == 0.0 ? ui->spinBox_SwPlato->value() * ui->spinBox_R_Factor->value() : 0.0);
    }
}

void DialogStammwuerze::on_spinBox_S_Temperatur_valueChanged(double)
{
    if (ui->spinBox_S_Temperatur->hasFocus())
    {
        ui->spinBox_SwPlato->setValue(QBerechnungen::densityAtX(ui->spinBox_S_SwPlato->value(), ui->spinBox_S_Temperatur->value(), 20));
        ui->spinBox_R_SwBrix->setValue(SWAnstellen == 0.0 ? ui->spinBox_SwPlato->value() * ui->spinBox_R_Factor->value() : 0.0);
    }
}

double DialogStammwuerze::brixToPlato()
{
    if (SWAnstellen == 0.0)
    {
        return ui->spinBox_R_SwBrix->value() / ui->spinBox_R_Factor->value();
    }
    else
    {

        double brix = ui->spinBox_R_SwBrix->value();
        double sw = SWAnstellen;
        double brixF = ui->spinBox_R_SwBrix->value() / ui->spinBox_R_Factor->value();

        //Standardformel
        QString formel = ui->comboBox_FormelBrixPlato->currentText();
        if (formel == "Standardformel")
        {
            double dichte =  1.001843-0.002318474*sw - 0.000007775*sw*sw - 0.000000034*sw*sw*sw + 0.00574*brix + 0.00003344*brix*brix + 0.000000086*brix*brix*brix;
            return QBerechnungen::GetGradPlato(dichte);
        }

        else if (formel == "Terrill")
        {
           double dichte = 1 - 0.0044993*sw + 0.0117741*brixF + 0.000275806*sw*sw - 0.00127169*brixF*brixF - 0.00000727999*sw*sw*sw + 0.0000632929*brixF*brixF*brixF;
           return QBerechnungen::GetGradPlato(dichte);
        }

        else if (formel == "Terrill linear")
        {
            double dichte = 1.0000 - 0.00085683*sw + 0.0034941*brixF;
            return QBerechnungen::GetGradPlato(dichte);
        }

        // User Kleier -> Quelle: http://hobbybrauer.de/modules.php?name=eBoard&file=viewthread&tid=11943&page=2#pid129201
        else
        {
            double Ballingkonstante = 2.0665;
            //tatsächlicher Restextrakt
            double tr =(Ballingkonstante * brix - Gaerungskorrektur * sw)/(Ballingkonstante
                    * ui->spinBox_R_Factor->value() - Gaerungskorrektur);
            //Scheinbarer Restextrakt
            return tr * (1.22 + 0.001 * sw) - ((0.22 + 0.001 * sw) * sw);
        }
    }
}

void DialogStammwuerze::on_spinBox_R_SwBrix_valueChanged(double)
{
    if (ui->spinBox_R_SwBrix->hasFocus())
    {
        ui->spinBox_SwPlato->setValue(brixToPlato());
        if (ui->spinBox_S_Temperatur->value() == 20.0)
        {
            ui->spinBox_S_SwPlato->setValue(ui->spinBox_SwPlato->value());
            ui->spinBox_S_SwDichte->setValue(QBerechnungen::GetDichte(ui->spinBox_SwPlato->value()));
        }
        else
        {
            ui->spinBox_S_SwPlato->setValue(0.0);
            ui->spinBox_S_SwDichte->setValue(0.0);
        }
    }
}

void DialogStammwuerze::on_spinBox_R_Factor_valueChanged(double)
{
    if (ui->spinBox_R_Factor->hasFocus())
    {
        ui->spinBox_SwPlato->setValue(brixToPlato());
        ui->spinBox_S_Temperatur->setValue(20.0);
        ui->spinBox_S_SwPlato->setValue(ui->spinBox_SwPlato->value());
        ui->spinBox_S_SwDichte->setValue(QBerechnungen::GetDichte(ui->spinBox_SwPlato->value()));
    }
}

void DialogStammwuerze::on_comboBox_FormelBrixPlato_currentIndexChanged(const QString &)
{
    if (ui->comboBox_FormelBrixPlato->hasFocus())
    {
        ui->spinBox_SwPlato->setValue(brixToPlato());
        ui->spinBox_S_Temperatur->setValue(20.0);
        ui->spinBox_S_SwPlato->setValue(ui->spinBox_SwPlato->value());
        ui->spinBox_S_SwDichte->setValue(QBerechnungen::GetDichte(ui->spinBox_SwPlato->value()));
    }
}
