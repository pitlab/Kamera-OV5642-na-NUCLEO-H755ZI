//////////////////////////////////////////////////////////////////////////////
//
// Karta pomiarowa rozdzielnicy
// Modu� obs�ugi wy�wietlania
//
// (c) PitLab 2020
// http://www.pitlab.pl
//////////////////////////////////////////////////////////////////////////////
#include "lcd.h"
#include "sys_def.h"
#include <stdio.h>
#include <math.h>
#include "stm32h7xx_hal.h"



////////////////////////////////////////////////////////////////////////////////
// Inicjalizuje wyświetlacz
// Parametry:nic
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void InitDisplay(void)
{
	chRysujRaz = 1;
	LCD_init();
}


////////////////////////////////////////////////////////////////////////////////
// Wyświetl menu wyboru trybów pracy
// Parametry: chPozycja - bieżąca pozyja menu
// Zwraca: nowy tryb pracy
////////////////////////////////////////////////////////////////////////////////
unsigned char Menu(unsigned char chPozycja)
{
	if (chRysujRaz)
	{
		LCD_clear();
		setFont(MidFont);
		setColor(GREEN);
		sprintf(chNapis, "AutoPitLot  SysCLK = %lu MHz", HAL_RCC_GetSysClockFreq()/1000000);
		print(chNapis, 10, 0, 0);
		sprintf(chNapis, "v%d.%d.%d @ %s %s", WER_GLOWNA, WER_PODRZ, WER_REPO, build_date, build_time);	//numer wersji w repozytorium i czas kompilacji
		print(chNapis, 10, 20, 0);

		setColor(YELLOW);
		sprintf(chNapis, "Podglad kamery RGB");
		print(chNapis, 20, 40, 0);
		//sprintf(chNapis, "Magnetometr");
		sprintf(chNapis, "Setup 2");
		print(chNapis, 20, 60, 0);
		//sprintf(chNapis, "Analiza ramki Eth");
		sprintf(chNapis, "Histogram RGB565");
		print(chNapis, 20, 80, 0);
		sprintf(chNapis, "Histogram bitow obrazu");
		print(chNapis, 20, 100, 0);
		sprintf(chNapis, "Konwersja na czarno-biały");
		print(chNapis, 20, 120, 0);
		sprintf(chNapis, "Detekcja krawedzi");
		print(chNapis, 20, 140, 0);
		sprintf(chNapis, "...");
		print(chNapis, 20, 160, 0);
		sprintf(chNapis, "Fraktale");
		print(chNapis, 20, 180, 0);
		sprintf(chNapis, "Pomocy, kabelki!");
		print(chNapis, 20, 200, 0);


#define TP_KAM_SET2		1
#define TP_HIST_RGB		2
#define TP_HIST_BIT		3
#define TP_KAM_CB		4
#define TP_DET_KRAW		5
#define TP_


		/*sprintf(chNapis, "Kasuj flash");
		print(chNapis, 180, 40, 0);
		sprintf(chNapis, "Nic 1");
		print(chNapis, 180, 60, 0);
		sprintf(chNapis, "Nic 2");
		print(chNapis, 180, 80, 0);*/
		chRysujRaz = 0;
		InitFraktal(0);
	}

	//kasuj stary znacznik
	setColor(BLACK);
	if (chPozycja < SRODKOWA_POZYCJA_MENU)
	{
		x = 0;
		y = chPozycja * 20 + 40;
	}
	/*else		//obsługa prawej połowy ekranu
	{
		x = 160;
		y = (chPozycja - SRODKOWA_POZYCJA_MENU) * 20 + 40;
	}*/
	printChar('>', x, y);

	//rysuj znacznik pozycji menu
	setColor(CYAN);
	chPozycja++;
	//if (chPozycja >= OSTATNIA_POZYCJA_MENU)	//menu na prawej i lewej części ekranu
	if (chPozycja >= SRODKOWA_POZYCJA_MENU)		//menu po lewej części ekranu
		chPozycja = 0;

	//oblicz współrzędne znacznika
	if (chPozycja < SRODKOWA_POZYCJA_MENU)
	{
		x = 0;
		y = chPozycja * 20 + 40;
	}
	/*else		//obsługa prawej połowy ekranu
	{
		x = 160;
		y = (chPozycja - SRODKOWA_POZYCJA_MENU) * 20 + 40;
	}*/
	printChar('>', x, y);

	return chPozycja;
}



////////////////////////////////////////////////////////////////////////////////
// Rysuj timer włączenia pozycji menu
// Parametry: chTryb - bieżący tryb pracy
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void RysujMenuTimer(unsigned short sCzas)
{
	setColor(BLUE);
	fillRect(0, DISP_Y_SIZE-4, (int)sCzas, DISP_Y_SIZE);
	setColor(BLACK);
	fillRect((int)sCzas+1, DISP_Y_SIZE-4, DISP_X_SIZE, DISP_Y_SIZE);
}



////////////////////////////////////////////////////////////////////////////////
// Rysuj zawartość rejestru w podanym wierszu
// Parametry:
// *str - wskaźnik na string oisujcy rejestr
// dane - zawartość rejestru do wyświetleania jako hex
// pozY - współrzędne Y wiersza
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void WyswietlDane8(char *str, uint8_t dane, uint8_t pozY)
{
	setColor(GREEN);
	sprintf(chNapis, "%s: 0x%X", str, dane);
	print(chNapis, 10, pozY, 0);
}

void WyswietlDane32(char *str, uint32_t dane, uint8_t pozY)
{
	setColor(BLUE);
	sprintf(chNapis, "%s: %ld", str, dane);
	print(chNapis, 10, pozY, 0);
}

void WyswietlDaneFloat(char *str, float dane, uint8_t pozY)
{
	setColor(YELLOW);
	sprintf(chNapis, "%s: %.2f", str, dane);
	print(chNapis, 10, pozY, 0);
}



////////////////////////////////////////////////////////////////////////////////
// Rysuj zawartość rejestru w podanym wierszu
// Parametry:
// *str - wskaźnik na string oisujcy rejestr
// dane - zawartość rejestru do wyświetleania jako hex
// pozY - współrzędne Y wiersza
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void WyswietlKodBledu(uint8_t blad, uint8_t pozX, uint8_t pozY)
{
	if (blad)
	{
		setColor(RED);
		sprintf(chNapis, "Err: %d ", blad);
	}
	else
	{
		setColor(GREEN);
		sprintf(chNapis, "OK    ");
	}
	print(chNapis, pozX, pozY, 0);
}



////////////////////////////////////////////////////////////////////////////////
// zmierz czas liczenia fraktala Julii
// Parametry: nic
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void InitFraktal(unsigned char chTyp)
{
#define ITERATION	80
//#define IMG_CONSTANT	0.285
//#define REAL_CONSTANT	0.005
//#define IMG_CONSTANT	-0.73
//#define REAL_CONSTANT	0.19
#define IMG_CONSTANT	-0.1
#define REAL_CONSTANT	0.65

	switch (chTyp)
	{
	case 0:	fReal = 0.38; 	fImag = -0.1;	break;	//Julia
	case 1:	fX=-0.70; 	fY=0.60;	fZoom = -0.6;	chMnozPalety = 2;	break;		//ca�y fraktal - rotacja palety -0.7, 0.6, -0.6,
	case 2:	fX=-0.75; 	fY=0.18;	fZoom = -0.6;	chMnozPalety = 15;	break;		//dolina konika x=-0,75, y=0,1
	case 3:	fX= 0.30; 	fY=0.05;	fZoom = -0.6;	chMnozPalety = 43;	break;		//dolina s�onia x=0,25-0,35, y=0,05, zoom=-0,6..-40
	}


	//chMnozPalety = 43;		//8, 13, 21, 30, 34, 43, 48, 56, 61
}



////////////////////////////////////////////////////////////////////////////////
// zmierz czas liczenia fraktala Julii
// Parametry: nic
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void FraktalTest(unsigned char chTyp)
{
	unsigned int nCzas;

	nCzas = HAL_GetTick();
	switch (chTyp)
	{
	case 0:	GenerateJulia(DISP_X_SIZE, DISP_Y_SIZE, DISP_X_SIZE/2, DISP_Y_SIZE/2, 135, sFractalBuf);
			nCzas = MinalCzas(nCzas);
			sprintf(chNapis, "Julia: t=%dms, c=%.3f ", nCzas, fImag);
			fImag -= 0.002;
			break;

			//ca�y fraktal - rotacja palety
	case 1: GenerateMandelbrot(fX, fY, fZoom, 30, sFractalBuf);
			nCzas = MinalCzas(nCzas);
			sprintf(chNapis, "Mandelbrot: t=%dms z=%.1f, p=%d", nCzas, fZoom, chMnozPalety);
			chMnozPalety += 1;
			break;

			//dolina konika x=-0,75, y=0,1
	case 2: GenerateMandelbrot(fX, fY, fZoom, 30, sFractalBuf);
			nCzas = MinalCzas(nCzas);
			sprintf(chNapis, "Mandelbrot: t=%dms z=%.1f, p=%d", nCzas, fZoom, chMnozPalety);
			fZoom /= 0.9;
			break;

			//dolina s�onia x=0,25-0,35, y=0,05, zoom=-0,6..-40
	case 3: GenerateMandelbrot(fX, fY, fZoom, 30, sFractalBuf);
			nCzas = MinalCzas(nCzas);
			sprintf(chNapis, "Mandelbrot: t=%dms z=%.1f, p=%d", nCzas, fZoom, chMnozPalety);
			fZoom /= 0.9;
			break;
	}

	drawBitmap(0, 0, 320, 240, sFractalBuf);
	setFont(MidFont);
	setColor(GREEN);
	print(chNapis, 0, 224, 0);
}


////////////////////////////////////////////////////////////////////////////////
// wyświetl demo z fraktalami
// Parametry: nic
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void FraktalDemo(void)
{
	switch (chDemoMode)
	{
	case 0:	FraktalTest(0);		//rysuj fraktala Julii
		if (chLiczIter > 40)
		{
			chLiczIter = 0;
			chDemoMode++;
			InitFraktal(1);
		}
		break;

	case 1:	FraktalTest(1);		//rysuj fraktala Mandelbrota - cały fraktal rotacja kolorów
		if (chLiczIter > 60)
		{
			chLiczIter = 0;
			chDemoMode++;
			InitFraktal(2);
		}
		break;

	case 2:	FraktalTest(2);		//rysuj fraktala Mandelbrota - dolina konika
		if (chLiczIter > 20)
		{
			chLiczIter = 0;
			chDemoMode++;
			InitFraktal(3);
		}
		break;

	case 3:	FraktalTest(3);		//rysuj fraktala Mandelbrota - dolina słonia
		if (chLiczIter > 20)
		{
			chLiczIter = 0;
			chDemoMode++;
			InitFraktal(2);
		}
		break;

	case 4:	chDemoMode++;	break;

	case 5:	//LCD_Test();		chMode++;
		InitFraktal(0);
		chDemoMode = 0;	break;
	}
	chLiczIter++;
}



////////////////////////////////////////////////////////////////////////////////
// Generuj fraktal Julii
// Parametry:
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void GenerateJulia(unsigned short size_x, unsigned short size_y, unsigned short offset_x, unsigned short offset_y, unsigned short zoom, unsigned short * buffer)
{
	float tmp1, tmp2;
	float num_real, num_img;
	float radius;
	unsigned short i;
	unsigned short x,y;

	for (y=0; y<size_y; y++)
	{
		for (x=0; x<size_x; x++)
		{
			num_real = y - offset_y;
			num_real = num_real / zoom;
			num_img = x - offset_x;
			num_img = num_img / zoom;
			i=0;
			radius = 0;
			while ((i<ITERATION-1) && (radius < 2))
			{
				tmp1 = num_real * num_real;
				tmp2 = num_img * num_img;
				num_img = 2*num_real*num_img + fImag;
				num_real = tmp1 - tmp2 + fReal;
				radius = tmp1 + tmp2;
				i++;
			}
			/* Store the value in the buffer */
			buffer[x+y*size_x] = i*20;
		}
	}
}


#define CONTROL_SIZE_Y	128


////////////////////////////////////////////////////////////////////////////////
// Generuj fraktal Mandelbrota
// Parametry:
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void GenerateMandelbrot(float centre_X, float centre_Y, float Zoom, unsigned short IterationMax, unsigned short * buffer)
{
	double X_Min = centre_X - 1.0/Zoom;
	double X_Max = centre_X + 1.0/Zoom;
	double Y_Min = centre_Y - (DISP_Y_SIZE-CONTROL_SIZE_Y) / (DISP_X_SIZE * Zoom);
	double Y_Max = centre_Y + (DISP_Y_SIZE-CONTROL_SIZE_Y) / (DISP_X_SIZE * Zoom);
	double dx = (X_Max - X_Min) / DISP_X_SIZE;
	double dy = (Y_Max - Y_Min) / (DISP_Y_SIZE-CONTROL_SIZE_Y) ;
	double y = Y_Min;
	unsigned short j, i;
	int n;
	//double c;
	double x, Zx, Zy, Zx2, Zy2, Zxy;

	//for (j = 0; j < (DISP_Y_SIZE-CONTROL_SIZE_Y); j++)
	for (j=0; j<DISP_Y_SIZE; j++)
	{
		x = X_Min;
		for (i = 0; i < DISP_X_SIZE; i++)
		{
			Zx = x;
			Zy = y;
			n = 0;
			while (n < IterationMax)
			{
				Zx2 = Zx * Zx;
				Zy2 = Zy * Zy;
				Zxy = 2.0 * Zx * Zy;
				Zx = Zx2 - Zy2 + x;
				Zy = Zxy + y;
				if(Zx2 + Zy2 > 16.0)
				{
					break;
				}
				n++;
			}
			x += dx;

			buffer[i+j*DISP_X_SIZE] = n*chMnozPalety;
		}
		y += dy;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Konwersja kolorów z HSV na RBG
// Parametry:
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void HSV2RGB(float hue, float sat, float val, float *red, float *grn, float *blu)
{
	int i;
	float f, p, q, t;
	if(val==0)
	{
		red=0;
		grn=0;
		val=0;
	}
	else
	{
		hue/=60;
		i = floor(hue);
		f = hue-i;
		p = val*(1-sat);
		q = val*(1-(sat*f));
		t = val*(1-(sat*(1-f)));
		if (i==0)
		{
			*red=val;
			*grn=t;
			*blu=p;
		}
		else if (i==1) {*red=q; 	*grn=val; 	*blu=p;}
		else if (i==2) {*red=p; 	*grn=val; 	*blu=t;}
		else if (i==3) {*red=p; 	*grn=q; 	*blu=val;}
		else if (i==4) {*red=t; 	*grn=p; 	*blu=val;}
		else if (i==5) {*red=val; 	*grn=p; 	*blu=q;}
	}
}



////////////////////////////////////////////////////////////////////////////////
// Liczy upływ czasu
// Parametry: nStart - licznik czasu na na początku pomiaru
// Zwraca: ilość czasu w setkach us jaki upłynął do podanego czasu startu
////////////////////////////////////////////////////////////////////////////////
unsigned int MinalCzas(unsigned int nStart)
{
	unsigned int nCzas, nCzasAkt;

	nCzasAkt = HAL_GetTick();
	if (nCzasAkt >= nStart)
		nCzas = nCzasAkt - nStart;
	else
		nCzas = 0xFFFFFFFF - nStart + nCzasAkt;
	return nCzas;
}



////////////////////////////////////////////////////////////////////////////////
// Wypisz kod błędu na ekranie
// Parametry: chErr - kod błędu
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void DispErr(unsigned char chErr)
{
	setFont(MidFont);
	setColor(RED);
	sprintf(chNapis, "Error %d (0x%X)", chErr, chErr);
	print(chNapis, 0, 0, 0);
}



////////////////////////////////////////////////////////////////////////////////
// wyświetla komunikat o sposobie podłączenia
// Parametry:
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void WyswietlPomoc(void)
{
	if (chRysujRaz)
	{
		setFont(MidFont);
		setColor(YELLOW);
		sprintf(chNapis, "Kamera DCMI");
		print(chNapis, 0, 0*20, 0);
		setColor(GREEN);
		sprintf(chNapis, "XCLK (PF9): CN11.56");
		print(chNapis, 0, 1*20, 0);
		sprintf(chNapis, "PIXCLK (PA6): CN12.14");
		print(chNapis, 0, 2*20, 0);
		sprintf(chNapis, "HSYNC (PA4): CN11.32");
		print(chNapis, 0, 3*20, 0);
		sprintf(chNapis, "VSYNC (PB7): CN11.21");
		print(chNapis, 0, 4*20, 0);
		sprintf(chNapis, "SCK (PF14): CN12.50");
		print(chNapis, 0, 5*20, 0);
		sprintf(chNapis, "SDA (PF15): CN12.60");
		print(chNapis, 0, 6*20, 0);
		sprintf(chNapis, "PWDN (PF11): CN12.62");
		print(chNapis, 0, 7*20, 0);
	}
}

////////////////////////////////////////////////////////////////////////////////
// wyświetla okno z wynikami analizy ramki eth
// Parametry:
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void WyswietlAnalizeEth(uint8_t* chAdrEthNad, uint8_t* chAdrIPNad)
{
	if (chRysujRaz)
		{
			LCD_clear();
			setFont(MidFont);
			setColor(GREEN);
			sprintf(chNapis, "Analiza ramki Ethernet");
			print(chNapis, 10, 0, 0);
			chRysujRaz = 0;
		}
		setColor(YELLOW);
		sprintf(chNapis, "Adr Eth Nad: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X", *(chAdrEthNad+0), *(chAdrEthNad+1), *(chAdrEthNad+2), *(chAdrEthNad+3), *(chAdrEthNad+4), *(chAdrEthNad+6));	//adres ramki
		print(chNapis, 20, 20, 0);
		sprintf(chNapis, "Adr IP Nad: %.3d.%.3d.%.3d,%.3d", *(chAdrIPNad+0), *(chAdrIPNad+1), *(chAdrIPNad+2), *(chAdrIPNad+3));
		print(chNapis, 20, 40, 0);
		/*sprintf(chNapis, "Podglad kamery RAW");
		print(chNapis, 20, 60, 0);
		sprintf(chNapis, "Setup 1");
		print(chNapis, 20, 80, 0);
		sprintf(chNapis, "Setup 2");
		print(chNapis, 20, 100, 0);
		sprintf(chNapis, "Setup 3");
		print(chNapis, 20, 120, 0);
		sprintf(chNapis, "Setup 4");
		print(chNapis, 20, 140, 0);
		sprintf(chNapis, "Setup 5");
		print(chNapis, 20, 160, 0);
		sprintf(chNapis, "Fraktale");
		print(chNapis, 20, 180, 0);
		sprintf(chNapis, "Pomocy, kabelki!");
		print(chNapis, 20, 200, 0);*/

}
