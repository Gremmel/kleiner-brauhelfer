#include "berechnungen.h"
#include <QtGlobal>
#include <math.h>
#include <QDebug>

// Tabelle mit Farbwertzuordnung srm -> RGB
int aFarbe[300][3] = {
	{250,250,210},
	{250,250,204},
	{250,250,199},
	{250,250,193},
	{250,250,188},
	{250,250,182},
	{250,250,177},
	{250,250,171},
	{250,250,166},
	{250,250,160},
	{250,250,155},
	{250,250,149},
	{250,250,144},
	{250,250,138},
	{250,250,133},
	{250,250,127},
	{250,250,122},
	{250,250,116},
	{250,250,111},
	{250,250,105},
	{250,250,100},
	{250,250,94},
	{250,250,89},
	{250,250,83},
	{250,250,78},
	{249,250,72},
	{248,249,67},
	{247,248,61},
	{246,247,56},
	{245,246,50},
	{244,245,45},
	{243,244,45},
	{242,242,45},
	{241,240,46},
	{240,238,46},
	{239,236,46},
	{238,234,46},
	{237,232,47},
	{236,230,47},
	{235,228,47},
	{234,226,47},
	{233,224,48},
	{232,222,48},
	{231,220,48},
	{230,218,48},
	{229,216,49},
	{228,214,49},
	{227,212,49},
	{226,210,49},
	{225,208,50},
	{224,206,50},
	{223,204,50},
	{222,202,50},
	{221,200,51},
	{220,198,51},
	{219,196,51},
	{218,194,51},
	{217,192,52},
	{216,190,52},
	{215,188,52},
	{214,186,52},
	{213,184,53},
	{212,182,53},
	{211,180,53},
	{210,178,53},
	{209,176,54},
	{208,174,54},
	{207,172,54},
	{206,170,54},
	{205,168,55},
	{204,166,55},
	{203,164,55},
	{202,162,55},
	{201,160,56},
	{200,158,56},
	{200,156,56},
	{199,154,56},
	{199,152,56},
	{198,150,56},
	{198,148,56},
	{197,146,56},
	{197,144,56},
	{196,142,56},
	{196,141,56},
	{195,140,56},
	{195,139,56},
	{194,139,56},
	{194,138,56},
	{193,137,56},
	{193,136,56},
	{192,136,56},
	{192,135,56},
	{192,134,56},
	{192,133,56},
	{192,133,56},
	{192,132,56},
	{192,131,56},
	{192,130,56},
	{192,130,56},
	{192,129,56},
	{192,128,56},
	{192,127,56},
	{192,127,56},
	{192,126,56},
	{192,125,56},
	{192,124,56},
	{192,124,56},
	{192,123,56},
	{192,122,56},
	{192,121,56},
	{192,121,56},
	{192,120,56},
	{192,119,56},
	{192,118,56},
	{192,118,56},
	{192,117,56},
	{192,116,56},
	{192,115,56},
	{192,115,56},
	{192,114,56},
	{192,113,56},
	{192,112,56},
	{192,112,56},
	{192,111,56},
	{192,110,56},
	{192,109,56},
	{192,109,56},
	{192,108,56},
	{191,107,56},
	{190,106,56},
	{189,106,56},
	{188,105,56},
	{187,104,56},
	{186,103,56},
	{185,103,56},
	{184,102,56},
	{183,101,56},
	{182,100,56},
	{181,100,56},
	{180,99,56},
	{179,98,56},
	{178,97,56},
	{177,97,56},
	{175,96,55},
	{174,95,55},
	{172,94,55},
	{171,94,55},
	{169,93,54},
	{168,92,54},
	{167,91,54},
	{165,91,54},
	{164,90,53},
	{162,89,53},
	{161,88,53},
	{159,88,53},
	{158,87,52},
	{157,86,52},
	{155,85,52},
	{154,85,52},
	{152,84,51},
	{151,83,51},
	{149,82,51},
	{148,82,51},
	{147,81,50},
	{145,80,50},
	{144,79,50},
	{142,78,50},
	{141,77,49},
	{139,76,49},
	{138,75,48},
	{137,75,47},
	{135,74,47},
	{134,73,46},
	{132,72,45},
	{131,72,45},
	{129,71,44},
	{128,70,43},
	{127,69,43},
	{125,69,42},
	{124,68,41},
	{122,67,41},
	{121,66,40},
	{119,66,39},
	{118,65,39},
	{117,64,38},
	{115,63,37},
	{114,63,37},
	{112,62,36},
	{111,61,35},
	{109,60,34},
	{108,60,33},
	{107,59,32},
	{105,58,31},
	{104,57,29},
	{102,57,28},
	{101,56,27},
	{99,55,26},
	{98,54,25},
	{97,54,24},
	{95,53,23},
	{94,52,21},
	{92,51,20},
	{91,51,19},
	{89,50,18},
	{88,49,17},
	{87,48,16},
	{85,48,15},
	{84,47,13},
	{82,46,12},
	{81,45,11},
	{79,45,10},
	{78,44,9},
	{77,43,8},
	{75,42,9},
	{74,42,9},
	{72,41,10},
	{71,40,10},
	{69,39,11},
	{68,39,11},
	{67,38,12},
	{65,37,12},
	{64,36,13},
	{62,36,13},
	{61,35,14},
	{59,34,14},
	{58,33,15},
	{57,33,15},
	{55,32,16},
	{54,31,16},
	{52,30,17},
	{51,30,17},
	{49,29,18},
	{48,28,18},
	{47,27,19},
	{45,27,19},
	{44,26,20},
	{42,25,20},
	{41,24,21},
	{39,24,21},
	{38,23,22},
	{37,22,21},
	{37,22,21},
	{36,22,21},
	{36,21,20},
	{35,21,20},
	{35,21,20},
	{34,20,19},
	{34,20,19},
	{33,20,19},
	{33,19,18},
	{32,19,18},
	{32,19,18},
	{31,18,17},
	{31,18,17},
	{30,18,17},
	{30,17,16},
	{29,17,16},
	{29,17,16},
	{28,16,15},
	{28,16,15},
	{27,16,15},
	{27,15,14},
	{26,15,14},
	{26,15,14},
	{25,14,13},
	{25,14,13},
	{24,14,13},
	{24,13,12},
	{23,13,12},
	{23,13,12},
	{22,12,11},
	{22,12,11},
	{21,12,11},
	{21,11,10},
	{20,11,10},
	{20,11,10},
	{19,10,9},
	{19,10,9},
	{18,10,9},
	{18,9,8},
	{17,9,8},
	{17,9,8},
	{16,8,7},
	{16,8,7},
	{15,8,7},
	{15,7,6},
	{14,7,6},
	{14,7,6},
	{13,6,5},
	{13,6,5},
	{12,6,5},
	{12,5,4},
	{11,5,4},
	{11,5,4},
	{10,4,3},
	{10,4,3},
	{9,4,3},
	{9,3,2},
	{8,3,2},
	{8,3,2}
};


// Tabelle Bittersoffausbeute 
double aBitterstoffausbeute[19][14] = {
	{ 0,   5,  10,   15,   20,   30,   40,   50,   60,   70,   80,   90,  100,  120},
	{ 8, 5.4, 9.9, 13.5, 16.5, 20.9, 23.8, 25.8, 27.2, 28.1, 28.7, 29.1, 29.3, 29.6},
	{ 9, 5.2, 9.5,   13, 15.9, 20.1,   23, 24.9, 26.2,   27, 27.6,   28, 28.3, 28.6},
	{10,   5, 9.1, 12.5, 15.3, 19.4, 22.1,   24, 25.2, 26.1, 26.6,   27, 27.2, 27.5},
	{11, 4.8, 8.8, 12.1, 14.7, 18.7, 21.3, 23.1, 24.3, 25.1, 25.6,   26, 26.2, 26.5},
	{12, 4.7, 8.5, 11.6, 14.2,   18, 20.5, 22.3, 23.4, 24.2, 24.7,   25, 25.3, 25.5},
	{13, 4.5, 8.2, 11.2, 13.7, 17.3, 19.8, 21.4, 22.5, 23.3, 23.8, 24.1, 24.3, 24.6},
	{14, 4.3, 7.9, 10.8, 13.1, 16.7,   19, 20.6, 21.7, 22.4, 22.9, 23.2, 23.4, 23.7},
	{15, 4.2, 7.6, 10.4, 12.6,   16, 18.3, 19.9, 20.9, 21.6,   22, 22.3, 22.5, 22.8},
	{16,   4, 7.3,   10, 12.2, 15.4, 17.6, 19.1, 20.1, 20.7, 21.2, 21.5, 21.7, 21.9},
	{17, 3.9,   7,  9.6, 11.7, 14.8,   17, 18.4, 19.3,   20, 20.4, 20.7, 20.9, 21.1},
	{18, 3.7, 6.7,  9.2, 11.2, 14.3, 16.3, 17.7, 18.6, 19.2, 19.6, 19.9, 20.1, 20.3},
	{19, 3.6, 6.5,  8.9, 10.8, 13.7, 15.7,   17, 17.9, 18.4, 18.8, 19.1, 19.3, 19.5},
	{20, 3.4, 6.2,  8.5, 10.4, 13.2, 15.1, 16.3, 17.2, 17.7, 18.1, 18.3, 18.5, 18.7},
	{21, 3.3,   6,  8.2,   10, 12.7, 14.5, 15.7, 16.5,   17, 17.4, 17.6, 17.8,   18},
	{22, 3.2, 5.7,  7.9,  9.6, 12.2, 13.9, 15.1, 15.8, 16.4, 16.7, 16.9, 17.1, 17.3},
	{23,   3, 5.5,  7.5,  9.2, 11.7, 13.3, 14.5, 15.2, 15.7,   16, 16.2, 16.4, 16.6},
	{24, 2.9, 5.3,  7.2,  8.8, 11.2, 12.8, 13.9, 14.6, 15.1, 15.4, 15.6, 15.8, 15.9},
	{25, 2.8, 5.1,  6.9,  8.5, 10.8, 12.3, 13.3,   14, 14.5, 14.8,   15, 15.1, 15.3}
};

// density data from "Zuckertechniker-Taschenbuch, Albert Bartens Verlage, Berlin, 1966, 7. Auflage"
double density_tbl[11][9] = {
	/* 0      5     10     15     20     25     30     35     40     g/100g */
	{1.0002,1.0204,1.0418,1.0632,1.0851,1.1088,1.1323,1.1574,1.1840}, /*  0°C, extrapolated, does not make much sense below 4°C */
	{0.9997,1.0196,1.0402,1.0614,1.0835,1.1064,1.1301,1.1547,1.1802}, /* 10°C */
	{0.9982,1.0178,1.0381,1.0591,1.0810,1.1035,1.1270,1.1513,1.1764}, /* 20°C */
	{0.9957,1.0151,1.0353,1.0561,1.0777,1.1000,1.1232,1.1473,1.1723}, /* 30°C */
	{0.9922,1.0116,1.0316,1.0522,1.0737,1.0958,1.1189,1.1428,1.1676}, /* 40°C */
	{0.9881,1.0072,1.0271,1.0477,1.0690,1.0910,1.1140,1.1377,1.1624}, /* 50°C */
	{0.9832,1.0023,1.0221,1.0424,1.0636,1.0856,1.1085,1.1321,1.1568}, /* 60°C */
	{0.9778,0.9968,1.0165,1.0368,1.0579,1.0798,1.1026,1.1262,1.1507}, /* 70°C */
	{0.9718,0.9908,1.0104,1.0306,1.0517,1.0735,1.0963,1.1198,1.1443}, /* 80°C */
	{0.9653,0.9842,1.0038,1.0240,1.0450,1.0669,1.0896,1.1130,1.1375}, /* 90°C */
	/*{0.9591,0.9780,0.9975,1.0176,1.0386,1.0606,1.0832,1.1065,1.1309}     99°C, original data, extrapolated 100°C is used instead */
	{0.9584,0.9773,0.9968,1.0169,1.0379,1.0599,1.0825,1.1058,1.1301}  /*100°C, extrapolated */
};

//
QBerechnungen::QBerechnungen(  ) 
{

}
//

double QBerechnungen::GetGesammtSchuettung(double cWunschmenge, 
  double cSollStammwuerze, double cSudhausausbeute)
{
	SollStammwuerze = double(qRound(cSollStammwuerze*100))/100;
	Wunschmenge = cWunschmenge;
	Sudhausausbeute = cSudhausausbeute;
	
	//SollStammwürze %°P in Dichte umrechnen
	DichteSollStammwuerze = GetDichte(SollStammwuerze); 
	
	//Masse der Würze in KG
	GesammtmasseWuerze = Wunschmenge * (DichteSollStammwuerze);
	
	//Erforderliche Extraktmenge in Gramm
	GesammtExtraktmenge = GesammtmasseWuerze * SollStammwuerze * 10;
	
	//Praktisch genutzter Extraktgehalt in g/KG
	EffektiverExtraktgehalt = 10 * Sudhausausbeute;
	
	GesammtSchuettung = GesammtExtraktmenge / EffektiverExtraktgehalt;
	
	return(GesammtSchuettung);
}


double QBerechnungen::GetDichte(double GradPlato)
{
	// Formel umgestellt aus Quelle: http://www.realbeer.com/spencer/attenuation.html
	double a,b,c,d;
	a = 205.347;
	b = 668.72;
	c = 463.37 + GradPlato;
	d = 4 * a * c;
	return (-b + sqrt(b * b - d)) / (2 * a) * -1;
}


double QBerechnungen::GetGradPlato(double Dichte)
{
	// Quelle: http://www.realbeer.com/spencer/attenuation.html
	return 668.72 * Dichte - 463.37 - 205.347 * Dichte * Dichte;
}


double QBerechnungen::GetHauptguss(double faktor)
{
	//Hauptguss Berechnen
	Hauptguss = GesammtSchuettung * faktor;
	return( Hauptguss );
}


double QBerechnungen::GetNachguss(double verdampfungsziffer, int kochdauer, double korrektur)
{
  //Berechnung aus "Berechnungen zur Hersetellung eines Kleinsudes" pdf
	//Berechne Nachguss
	PfanneVoll = Wunschmenge + Wunschmenge * ((verdampfungsziffer * kochdauer) / (60*100));
  //qDebug() << "PfanneVoll: " << PfanneVoll;
  double treberwasser = GesammtSchuettung * 0.96;
  //qDebug() << "treberwasser: " << treberwasser;
	double brauwasser = PfanneVoll + treberwasser;
  //qDebug() << "brauwasser: " << brauwasser;
  Nachguss = brauwasser - Hauptguss + korrektur;
  //qDebug() << "Nachguss: " << Nachguss;

	PfanneVoll = BerVolumenWasser(20, 99, PfanneVoll);

	return Nachguss;
}


void QBerechnungen::BerHopfenGewichtProzent(double* resultMenge,
															double* Alphaprozent,
															double* Mengeprozent,
															int* Kochdauer,
															bool* Pellets,
															double SollIBU,
															int ArrayAnzahl,
															bool* Vorderwuerzehopfung,
															double* Ausbeute,
															double* IBUAnteil)
{
	//Differenz der GesammtIBU und der SollIBU
	double diffIBU = 0;
	//Zähler für ausstieg falls die berechnung fehlschlägt
	int zaehler = 0;
	//summe der IBUWerte
	double summeIBU;
	//erster Schleifendurchlauf
	bool erster = true;
	//IBU ausgleichswert
	double ausgleich = 0;
	
	//Ausbeute in Array schreiben
	for (int i = 0; i < ArrayAnzahl; i++){
//		qDebug() << "Kochdauer[i] "<< i << " " << Kochdauer[i];
		Ausbeute[i] = GetHopfenausbeute(Kochdauer[i], SollStammwuerze, Pellets[i]);
		//Bei Vorderwürzehopfung die Ausbeute um 10% verrigern
		if (Vorderwuerzehopfung[i]){
			Ausbeute[i] = Ausbeute[i] * 0.9;
		}
	}

	//Mengen solange berechnen bis IBU wert nahezu stimmt
	while ( ((diffIBU > 0.01) || (diffIBU < -0.01) || (erster)) && (zaehler < 1000)){
		erster = false;
		zaehler++;
		//Mengen ermitteln
		summeIBU = 0;
		for (int i = 0; i < ArrayAnzahl; i++){
			if (i == 0)
				resultMenge[i] = ((SollIBU - ausgleich) * Wunschmenge * 10) / (Alphaprozent[i] * Ausbeute[i]);
			else {
				resultMenge[i] = resultMenge[0] / Mengeprozent[0] * Mengeprozent[i];
//				qDebug() << "reusltMenge " << i << "  " << resultMenge[i] << " resultMenge[0] " << resultMenge[0] << " Mengeprozent[0] " << Mengeprozent[0] << " prozent[i] " << Mengeprozent[i];
			}
		}
		//IBU Anteile ermitteln
//		qDebug() << "--------------------------------";
//		qDebug() << "zaehler: " << zaehler;
		for (int i = 0; i < ArrayAnzahl; i++){
			IBUAnteil[i] = resultMenge[i] * Alphaprozent[i] * Ausbeute[i] / 10 / Wunschmenge;
//			qDebug() << "index: " << i;
//			qDebug() << "resultMenge: " << resultMenge[i];
//			qDebug() << "IBU Anteil: " << IBUAnteil[i];
			summeIBU += IBUAnteil[i];
		}
		diffIBU = summeIBU - SollIBU;
//		qDebug() << "IBU Summe: " << summeIBU;
//		qDebug() << "IBU Abweichung: " << diffIBU;
//		qDebug() << "IBU ausgleich: " << ausgleich;
		//IBU Korrekturwert für die Erste Gabe festlegen
		if (diffIBU > 10)
		  ausgleich += 1;
		else if (diffIBU > 1)
			ausgleich += 0.1;
		else if (diffIBU > 0)
		  ausgleich += 0.01;
		else if (diffIBU < -0.1)
		  ausgleich -= 0.001;
		else if (diffIBU < -0.01)
		  ausgleich -= 0.0001;
	}
}

void QBerechnungen::BerHopfenIBUProzent(double *resultMenge,
																				double *Alphaprozent,
																				double *Mengeprozent,
																				int *Kochdauer,
																				bool *Pellets,
																				double SollIBU,
																				int ArrayAnzahl,
																				bool *Vorderwuerzehopfung,
																				double *Ausbeute,
																				double *IBUAnteil)
{
	//Ausbeute in Array schreiben
	for (int i = 0; i < ArrayAnzahl; i++){
		Ausbeute[i] = GetHopfenausbeute(Kochdauer[i], SollStammwuerze, Pellets[i]);
		//Bei Vorderwürzehopfung die Ausbeute um 10% verrigern
		if (Vorderwuerzehopfung[i]){
			Ausbeute[i] = Ausbeute[i] * 0.9;
		}
		if (Ausbeute[i] < 0.1)
			Ausbeute[i]=0.1;
	}
	//IBU Anteil ausrechnen
	for (int i = 0; i < ArrayAnzahl; i++){
		IBUAnteil[i] = SollIBU / 100 * Mengeprozent[i];
	}
	//Menge anhand IBU Anteil berechnen
	//qDebug() << "Wunschmenge:" << Wunschmenge;
	//qDebug() << "SollIBU:" << SollIBU;
	for (int i = 0; i < ArrayAnzahl; i++){
		resultMenge[i] = (IBUAnteil[i] * Wunschmenge * 10) / (Alphaprozent[i] * Ausbeute[i]);
	}
	
}

double QBerechnungen::GetHopfenausbeute(double Kochzeit, double Stammwuerze, bool pellets)
{
	//Spalte aus Tabelle ermitteln
	int Spalte = 0;
	int LetzteSpalte = 13;
	bool SpalteGenau = false;
	//Kochzeit ist kleiner erster Wert
	if (aBitterstoffausbeute[0][1] > Kochzeit){
		Spalte = 0;
	}
	//Kochzeit ist größter letzter Wert
	else if (aBitterstoffausbeute[0][LetzteSpalte] <= Kochzeit){
		SpalteGenau = true;
		Spalte = LetzteSpalte;
	}
	else {
		for (int i = 0; i < LetzteSpalte; i++){
			//Wert passt genau auf die Spalte
			if (aBitterstoffausbeute[0][i] == Kochzeit){
				SpalteGenau = true;
				Spalte = i;
				i = LetzteSpalte + 1;
			}
			//Wert Liegt zwischen zwei Spalten
			else if ((aBitterstoffausbeute[0][i] < Kochzeit) && (aBitterstoffausbeute[0][i + 1] > Kochzeit) ){
				Spalte = i;
				i = LetzteSpalte + 1;
			}
		}
	}
	
	//Zeile aus Tabelle ermitteln
	int Zeile = 0;
	int LetzteZeile = 18;
	bool ZeileGenau = false;
	//Stammwürze liegt unter erstem Wert
	if (aBitterstoffausbeute[1][0] > Stammwuerze){
		Zeile = 0;
	}
	//Stammwürze ist größer wie letzter Wert
	else if (aBitterstoffausbeute[LetzteZeile][0] <= Stammwuerze){
		ZeileGenau = true;
		Zeile = LetzteZeile;
	}
	else {
		for (int i = 0; i < LetzteZeile; i++){
			//Wert passt genau auf die Spalte
			if (aBitterstoffausbeute[i][0] == Stammwuerze){
				ZeileGenau = true;
				Zeile = i;
				i = LetzteZeile + 1;
			}
			//Wert Liegt zwischen zwei Spalten
			else if ((aBitterstoffausbeute[i][0] < Stammwuerze) && (aBitterstoffausbeute[i + 1][0] > Stammwuerze) ){
				Zeile = i;
				i = LetzteZeile + 1;
			}
		}
	}
	
	//Wert ermitteln
	
	//Zeile und Spalte passt genau
	if (ZeileGenau && SpalteGenau){
		if (pellets)
			return aBitterstoffausbeute[Zeile][Spalte] * 1.1;
		else
			return aBitterstoffausbeute[Zeile][Spalte];
	}
	//Spalte passt genau
	else if (!ZeileGenau && SpalteGenau){
		//Stammwürze unterschreitet Tabllenwert
		if (Zeile == 0){
			if (pellets)
				return aBitterstoffausbeute[1][Spalte] * 1.1;
			else
				return aBitterstoffausbeute[1][Spalte];
		}
		//Stammwüzre ist im Tabellenbereich
		else{
			double w1, w2, diff1, diff2, a1, a2, divisor;
			w1 = aBitterstoffausbeute[Zeile][Spalte];
			w2 = aBitterstoffausbeute[Zeile + 1][Spalte];
			diff1 = w2 - w1;
			a1 = aBitterstoffausbeute[Zeile][0];
			a2 = aBitterstoffausbeute[Zeile + 1][0];
			diff2 = a2 - a1;
			divisor = diff2 / (Stammwuerze - a1);
			if (pellets)
				return (w1 + diff1 / divisor) * 1.1;
			else
				return w1 + diff1 / divisor;
		}
	}
	//Zeile passt genau
	else if (ZeileGenau && !SpalteGenau){
		double w1, w2, diff1, diff2, a1, a2, divisor;
		if (Spalte == 0)
			w1 = 0;
		else
		  w1 = aBitterstoffausbeute[Zeile][Spalte];
		w2 = aBitterstoffausbeute[Zeile][Spalte + 1];
		diff1 = w2 - w1;
		a1 = aBitterstoffausbeute[0][Spalte];
		a2 = aBitterstoffausbeute[0][Spalte + 1];
		diff2 = a2 - a1;
		divisor = diff2 / (Kochzeit - a1);
		if (pellets)
			return (w1 + diff1 / divisor) * 1.1;
		else
			return w1 + diff1 / divisor;
	}
	//wenn Spalte und Zeile nicht genau passt
	else {
		double WertZeile1, WertZeile2;
		//Wert 1 zwischen Zeilen
		double w1, w2, diff1, diff2, a1, a2, divisor;
		if (Spalte == 0){
			w1 = 0;
			w2 = 0;
		}
		else {
			w1 = aBitterstoffausbeute[Zeile][Spalte];
			w2 = aBitterstoffausbeute[Zeile + 1][Spalte];
		}
		diff1 = w2 - w1;
		a1 = aBitterstoffausbeute[Zeile][0];
		a2 = aBitterstoffausbeute[Zeile + 1][0];
		diff2 = a2 - a1;
		divisor = diff2 / (Stammwuerze - a1);
		WertZeile1 = w1 + diff1 / divisor;
		//Wert 2 zwischen Zeilen
		w1 = aBitterstoffausbeute[Zeile][Spalte + 1];
		w2 = aBitterstoffausbeute[Zeile + 1][Spalte + 1];
		diff1 = w2 - w1;
		a1 = aBitterstoffausbeute[Zeile][0];
		a2 = aBitterstoffausbeute[Zeile + 1][0];
		diff2 = a2 - a1;
		divisor = diff2 / (Stammwuerze - a1);
		WertZeile2 = w1 + diff1 / divisor;

		double ergebniss;
		//Wert den ermittelten Zeilenwerten
    w1 = WertZeile1;
		w2 = WertZeile2;
		diff1 = w2 - w1;
		a1 = aBitterstoffausbeute[0][Spalte];
		a2 = aBitterstoffausbeute[0][Spalte + 1];
		diff2 = a2 - a1;
		divisor = diff2 / (Kochzeit - a1);
		ergebniss = w1 + diff1 / divisor;
		
		if (pellets)
			return ergebniss * 1.1;
		else
			return ergebniss;
	}
}


double QBerechnungen::BerAlkohoVol(double SW, double SVP)
{
	//Vergärungsgrad scheinbar
	double vgs;
	vgs = GetScheinbarerEVG(SW, SVP);
	//Vergärbarer Zucker
	double Zucker;
	//SW = Stammwürze
	//SVP = Restextrakt (Schnellgärprobe)
	Zucker = SW - ( SW * vgs * 0.82 / 100);
	//Alkohol (Gewicht)
	double AlkGewicht;
	AlkGewicht = (SW - Zucker) / (2.0665 -1.0665 * SW / 100);
	//Alkohol (Volumen %)
	return AlkGewicht * GetDichte(SVP) / 0.7894;
	//return AlkGewicht * GetDichte(SVP) / 0.7894;

	//Berechnung alt
//	//Vergärbarer Zucker
//	double Zucker;
//	//SW = Stammwürze
//	//SVP = Restextrakt (Schnellgärprobe)
//	Zucker = 0.81 * (SW - SVP);
//	//Alkohol (Gewicht)
//	double AlkGewicht;
//	AlkGewicht = 0.5 * Zucker;
//	//Alkohol (Volumen %)
//	return AlkGewicht / 0.795;
}


double QBerechnungen::GetGruenschlauchzeitpunkt(double cSollCO2, double cTemperatur, double cRestextrakt)
{
	SollCO2 = cSollCO2;
	TemperaturJungbier = cTemperatur;
	RestextraktSchnellgaerprobe = cRestextrakt;
	double DichteRestextrakt = GetDichte(RestextraktSchnellgaerprobe);
	
	//CO2 Sättigung im Jungbier ausrechnen
	CO2SaettigungJungbier = 1.013 * ( pow(2.71828182845904, (-10.73797 + (2617.25 / (TemperaturJungbier + 273.15) ) ) ) ) * 10;

	//Benötigte Karbonisierung
	double BenoetigteKarbonisierung = SollCO2 - CO2SaettigungJungbier;
	
	//Erforderlicher vergärbarer Extrakt (Zucker) (Nach Endvergärung) g/l
	ErforderlicherZuckerEndvergoren = 2 * BenoetigteKarbonisierung;
	
	//Grünschlauchzeitpunkt
	Gruenschlauchzeitpunkt = RestextraktSchnellgaerprobe + (ErforderlicherZuckerEndvergoren / (8.192 * DichteRestextrakt));
	
	return Gruenschlauchzeitpunkt;

}

double QBerechnungen::GetPfanneVoll(double verdampfungsziffer, int kochdauer, double cWunschmenge)
{
  //Berechnung aus "Berechnungen zur Hersetellung eines Kleinsudes" pdf
  //Berechne Nachguss
  PfanneVoll = cWunschmenge + cWunschmenge * ((verdampfungsziffer * kochdauer) / (60*100));
  PfanneVoll = BerVolumenWasser(20, 99, PfanneVoll);
  return PfanneVoll;
}


double QBerechnungen::GetSpeiseGesammt(double SW, double SWJungbier, double Jungbiermenge, double SpeiseVerfuegbar, bool Spunden)
{

  //wenn gespundet wird muss keine Zucker und Speisemenge berechnet werden
  if (Spunden) {
    HaushaltszuckerGesammt = 0;
    ErforderlicheSpeiseGesammt = 0;
  }
  else {

    //tatsächlicher Restextrakt vor dem Abfüllen %°P
    RestextraktJungbier = SWJungbier;

    //erforderlicher vergärbarer Extrakt (Zucker) g/l
    double DichteRestextrakt = GetDichte(RestextraktSchnellgaerprobe);
    ErforderlicherZucker = (Gruenschlauchzeitpunkt - RestextraktJungbier) * 8.192 * DichteRestextrakt;

    //Dichte Stammwürze beim Anstellen
    double DichteStammwuerze = GetDichte(SW);

    //vergärbarer Extrakt in der Speise
    double vergaerbarerExtraktSpeise = ( SW - RestextraktSchnellgaerprobe) * 8.192 * DichteStammwuerze;

    //Erforderliche Speisemenge Gesammt ml
    ErforderlicheSpeiseGesammt = Jungbiermenge * ErforderlicherZucker / ( vergaerbarerExtraktSpeise - ErforderlicherZucker ) * 1000;

    //Weil die Speise ja auch noch aufkarbonisiert werden muss die menge nochmal verrechnen
    ErforderlicheSpeiseGesammt += (ErforderlicheSpeiseGesammt / 1000) * ErforderlicherZucker / ( vergaerbarerExtraktSpeise - ErforderlicherZucker ) * 1000;

    //wenn genügend Speise vorhanden ist
    if ( (SpeiseVerfuegbar * 1000) > ErforderlicheSpeiseGesammt){
      HaushaltszuckerGesammt = 0;
    }
    //wenn noch mit Zucker nachgeholfen werden muss
    else {
      HaushaltszuckerGesammt = (ErforderlicherZucker *  Jungbiermenge) / ErforderlicheSpeiseGesammt * (ErforderlicheSpeiseGesammt - (SpeiseVerfuegbar * 1000));
      ErforderlicheSpeiseGesammt = SpeiseVerfuegbar * 1000;
    }
  }

	return ErforderlicheSpeiseGesammt;
}


double QBerechnungen::GetHaushaltszuckerGesammt()
{
	return HaushaltszuckerGesammt;
}


double QBerechnungen::GetWasserVerschneidung(double SWSoll, double SWIst, double Menge)
{
    if (SWIst < SWSoll || SWSoll == 0.0)
        return 0.0;
    return Menge * (SWIst / SWSoll - 1);
}


double QBerechnungen::GetSpundungsdruck()
{
	return SollCO2 / ( ( pow( 2.71828182845904, ( -10.73797 + ( 2617.25 / ( TemperaturJungbier + 273.15 ) ) ) ) ) *10 ) -1.013;
}


// approximate the density function using ax^3+bx^2+cx+d and 
// interpolate for a given x
double QBerechnungen::interpolate(double x1,double y1,double x2,double y2,double x3,double y3,double x4,double y4,double x)
{
	// lagrange interpolation
	double y =
		y1 * (((x-x2)*(x-x3)*(x-x4)) / ((x1-x2)*(x1-x3)*(x1-x4))) +
		y2 * (((x-x1)*(x-x3)*(x-x4)) / ((x2-x1)*(x2-x3)*(x2-x4))) +
		y3 * (((x-x1)*(x-x2)*(x-x4)) / ((x3-x1)*(x3-x2)*(x3-x4))) +
		y4 * (((x-x1)*(x-x2)*(x-x3)) / ((x4-x1)*(x4-x2)*(x4-x3)));
	
	return y;
}


// interpolates the density values for a given temperature
void QBerechnungen::interpolateTemperature(double* result, double temperature)
{
	double lowest = floor(temperature / 10) - 1;
	if (0 > lowest) {
		lowest = 0;
	}
	//if (lowest > density_tbl.length - 4) {
	//  lowest = density_tbl.length - 4;
	//}
	
	int temp1 = lowest;
	int temp2 = lowest + 1;
	int temp3 = lowest + 2;
	int temp4 = lowest + 3;
	
	for (int i=0; i < 9; i++) {
		result[i] = interpolate(temp1*10, density_tbl[temp1][i], temp2*10, density_tbl[temp2][i], temp3*10, density_tbl[temp3][i], temp4*10, density_tbl[temp4][i], temperature);
	}

}


// reads the density values for a given temperature from the data table
// values for non existing temperatures are interpolated
double QBerechnungen::densityAtBaseTemp(double base, double plato)
{
	double density_base[9]; 
	interpolateTemperature(density_base, base);
	
	double lowest = floor(plato / 5) - 1;
	if (0 > lowest) {
		lowest = 0;
	} 
	//if (lowest > density_base.length - 4) {
	//  lowest = density_base.length - 4;
	//}
	
	int plato1 = lowest;
	int plato2 = lowest + 1;
	int plato3 = lowest + 2;
	int plato4 = lowest + 3;
	
	return interpolate(plato1*5, density_base[plato1], plato2*5, density_base[plato2], plato3*5, density_base[plato3], plato4*5, density_base[plato4], plato);
}


double QBerechnungen::densityAtX(double platoMeasure, double temperature, double calibrationTemp)
{
	double dAt20 = densityAtBaseTemp(calibrationTemp, platoMeasure);
	double tmp[9]; 
	interpolateTemperature(tmp, temperature);
	
	// find density interval
	int minimalIndex = 0;
	for (int i = 0; i < 9; i++) {
		if (dAt20 > tmp[i]) {
			minimalIndex = i;
		}
	}
	int lowest = minimalIndex - 1;
	if (0 > lowest) {
		lowest = 0;
	}
	//  if (lowest > tmp.length - 4) {
	//    lowest = tmp.length - 4;
	//  }
	
	int plato1 = lowest;
	int plato2 = lowest + 1;
	int plato3 = lowest + 2;
	int plato4 = lowest + 3;
	
	return interpolate(tmp[plato1], plato1*5, tmp[plato2], plato2*5, tmp[plato3], plato3*5, tmp[plato4], plato4*5, dAt20);

}


//Gibt die sudhausausbeute zurück
double QBerechnungen::GetSudhausausbeute(double SW, double Menge, double Schuettung)
{
  if ((SW > 0) && (Menge > 0) && (Schuettung > 0)) {
    double mixed;
    mixed = densityAtBaseTemp(20, SW) * SW;
    return mixed * Menge / Schuettung;
  }
  else {
    return 0;
  }
}


QColor QBerechnungen::GetFarbwert(double* aSchuettung, double* aFarbwerte, int Anzahl, double SW, int Korrektur)
{
	double d = 0;
	double gs = 0;
	for (int i=0; i < Anzahl; i++){
		d += aSchuettung[i] * aFarbwerte[i];
		gs += aSchuettung[i];
	}
	d = (d / gs) * SW / 10 + 2;
	
  //mit korrektur
  d += Korrektur;
  if (d<0) {
    Anzahl = 0;
    d=0;
  }

	double srm = d / 1.97;
  EBC = d;
	
	int index = qRound(srm *10);
	if (index > 300)
		index = 300;
	
	if (Anzahl == 0)
		return Qt::white;
	else
		return QColor::fromRgb(aFarbe[int(index)-1][0], aFarbe[int(index)-1][1], aFarbe[int(index)-1][2]);
}


double QBerechnungen::getEBC()
{
	return EBC;
}

QColor QBerechnungen::GetFarbwert(double EBC)
{
	double srm = EBC / 1.97;
	int index = qRound(srm *10);
	if (index > 300)
		index = 300;
	return QColor::fromRgb(aFarbe[int(index)-1][0], aFarbe[int(index)-1][1], aFarbe[int(index)-1][2]);
}

double QBerechnungen::BerCO2Gehalt(double druck, double temperatur)
{
	return (1.013 + druck) * pow(2.71828182845904,(-10.73797+(2617.25/(temperatur+273.15))))*10;
}

double QBerechnungen::GetIBU(double Wuerzemenge, double SW, double Hopfenmenge, double AlphaP, double Kochzeit, bool Pellets)
{
	double Ausbeute = GetHopfenausbeute(Kochzeit, SW, Pellets);
	double IBU = Hopfenmenge * AlphaP * Ausbeute / 10 / Wuerzemenge;
	return IBU;
}


double QBerechnungen::GetScheinbarerEVG(double SW, double SVP)
{
	// Scheinbarer EVG[%] = (Stammwürze [°P] - Restextrakt [°P] ) * 100 / Stammwürze [°P]
	return (SW - SVP) * 100 / SW;
}


double QBerechnungen::GetTatsaechlicherEVG(double SW, double SVP)
{
	// Scheinbarer EVG[%] = (Stammwürze [°P] - Restextrakt [°P] ) * 100 / Stammwürze [°P]
	//Tatsächlicher EVG[%] = Scheinbarer EVG[%] * 0,81
	return ((SW - SVP) * 100 / SW) * 0.81;
}


double QBerechnungen::BerVolumenWasser(double Temperatur1, double Temperatur2, double VolumenT1)
{
	//Volumen bei Temperatur 2 berechnen
	double DichteT1 = BerDichteWasser(Temperatur1);
	double DichteT2 = BerDichteWasser(Temperatur2);
	
	return (DichteT1 * VolumenT1) / DichteT2;
}

// Gibt die Dichte von Wasser zurück anhand der Übergebenen Temperatur
// Formel ist von: Dr.-Ing. Heiner Grimm, Clausthal-Zellerfeld
// Quelle: http://www.wissenschaft-technik-ethik.de
double QBerechnungen::BerDichteWasser(double Temperatur)
{
	//Formel: (a0+T*a1+T^2*a2+T^3*a3+T^4*a4+T^5*a5)/(1+T*b)
	double a0 = 999.83952;
	double a1 = 16.952577;
	double a2 = -0.0079905127;
	double a3 = -0.000046241757;
	double a4 = 0.00000010584601;
	double a5 = -0.00000000028103006;
	double b = 0.016887236;
	double t = Temperatur;
	
	double dichte = (a0 + t * a1 + pow(t,2) * a2 + pow(t,3) * a3 + pow(t,4) * a4 
			+ pow(t,5) * a5) / (1 + t * b);
	return dichte;
}

void QBerechnungen::setSollStammwuerze(double sw)
{
	SollStammwuerze = sw;
}

