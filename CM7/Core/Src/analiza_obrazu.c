//////////////////////////////////////////////////////////////////////////////
//
// Zestaw narzędzi do obróbki obrazu
//
// (c) PitLab 2024
// http://www.pitlab.pl
//////////////////////////////////////////////////////////////////////////////
#include "analiza_obrazu.h"
#include "stdlib.h"


////////////////////////////////////////////////////////////////////////////////
// Konwertuje dane RGB w formacie 565 znajdujace się w buforze obrazRGB565 do danych czarno-białych o głębi 7-bitowej
// Zapisuje kopię obrazu czarnobiałego w formacie wyświetlacza RGB565
// Parametry:
// [we] obrazRGB565* - wskaźnik na bufor[2*rozmiar] z obrazem kolorowym
// [wy] obrazCB* - wskaźnik na bufor[rozmiar] z obrazem czarno-białym
// [we] rozmiar - rozmiar bufora
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void KonwersjaRGB565doCB7(uint16_t *obrazRGB565, uint8_t *obrazCB, uint32_t rozmiar)
{
	uint16_t pix;
	for (uint32_t n=0; n<rozmiar; n++)
	{
		pix = *(obrazRGB565 + n);
		//bity czerwony i niebieski mają skalę 5-bitową (32), zielony 6-bitową (64).
		*(obrazCB + n) = (pix >> 11) + ((pix & 0x07E0) >> 5) + (pix & 0x1F);		//B32 + G64 + R32
	}
}



////////////////////////////////////////////////////////////////////////////////
// Konwertuje dane czarno-białe o głębi 7-bitowej na RGB565
// Parametry:
// [we] obrazCB* - wskaźnik na bufor[rozmiar] z obrazem czarno-białym
// [wy] obrazCB565* - wskaźnik na bufor[2*rozmiar] z obrazem czarno-białym
// rozmiar - rozmiar bufora
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void KonwersjaCB7doRGB565(uint8_t *obrazCB, uint16_t *obrazCB565, uint32_t rozmiar)
{
	uint16_t pixCB, pixRB, pixG;
	for (uint32_t n=0; n<rozmiar; n++)
	{
		pixCB = *(obrazCB+n);
		pixRB = pixCB >> 2;	//składowe: czerwona i niebieska
		pixG  = pixCB >> 1;	//składowa zielona
		*(obrazCB565 + n) = (pixRB << 11) + (pixG << 5) + pixRB;
	}
}



////////////////////////////////////////////////////////////////////////////////
// Wykonuje splot Robertsa do detekcji krawędzi na obrazie czarno-białym
// Są dwie macierze splotu robiace detekcję w poziomie i pionie
// [1  0] [ 0 1]
// [0 -1] [-1 0]
// Parametry:
// [we] obrazWe* - wskaźnik na bufor[rozmiar] z obrazem czarno-białym
// [wy] obrazWy* - wskaźnik na bufor[rozmiar] z obrazem  po detekcji (może być ten sam obszar co wejściowy)
// [we] szerokosc - szerokość obrazu (ilość pikseli w wierszu)
// [we] wysokosc - wysokość obrazu (ilość wierszy)
// [we] prog - próg odcięcia, wartość powyżej ktorej cokolwiek znajdzie sie w obrazie wyjściowym
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void DetekcjaKrawedziRoberts(uint8_t *obrazWe, uint8_t *obrazWy, uint16_t szerokosc, uint16_t wysokosc, uint8_t prog)
{
	uint16_t x, y;
	int pixWe[4], pixWy;

	*obrazWy = 0;	//inicjuj pierwszy piksel, bo pętla go nie ruszy
	for (y=0; y<wysokosc-1; y++)
	{
		for (x=1; x<szerokosc-1; x++)
		{
			pixWe[0] = *(obrazWe + (y*(szerokosc)) + x);
			pixWe[1] = *(obrazWe + (y*(szerokosc)) + x+1);

			pixWe[2] = *(obrazWe + ((y+1)*szerokosc) + x);
			pixWe[3] = *(obrazWe + ((y+1)*szerokosc) + x+1);

			pixWy = abs(pixWe[0] - pixWe[3]) + abs(pixWe[2] - pixWe[1]);
			if (pixWy <= prog)
				pixWy = 0;
			*(obrazWy + y*szerokosc + x) = pixWy;
		}
	}
}



////////////////////////////////////////////////////////////////////////////////
// Wykonuje splot Sobela do detekcji krawędzi na obrazie czarno-białym
// Są dwie macierze splotu robiace detekcję w poziomie i pionie
// [-1 0 1] [ 1  2  1]
// [-2 0 2] [ 0  0  0]
// [-1 0 1] [-1 -2 -1]
// Parametry:
// [we] obrazWe* - wskaźnik na bufor[rozmiar] z obrazem czarno-białym
// [wy] obrazWy* - wskaźnik na bufor[rozmiar] z obrazem po detekcji
// szerokosc - szerokość obrazu (ilość pikseli w wierszu)
// wysokosc - wysokość obrazu (ilość wierszy)
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void DetekcjaKrawedzi(uint8_t *obrazWe, uint8_t *obrazWy, uint16_t szerokosc, uint16_t wysokosc)
{
	uint16_t x, y;
	uint8_t pixWe[9], pixWy[9];

	//inicjuj dane wyjściowe, bo będziemy dodawali do tego
	for (uint32_t n=0; n<szerokosc*wysokosc; n++)
		*(obrazWy + n) = 0;

	for (y=1; y<wysokosc-1; y++)
	{
		for (x=1; x<szerokosc-1; x++)
		{
			pixWe[0] = *(obrazWe + (y*(szerokosc-1)) + x - 1);
			pixWe[1] = *(obrazWe + (y*(szerokosc-1)) + x);
			pixWe[2] = *(obrazWe + (y*(szerokosc-1)) + x + 1);

			pixWe[3] = *(obrazWe + (y*(szerokosc)) + x - 1);
			pixWe[4] = *(obrazWe + (y*(szerokosc)) + x);
			pixWe[5] = *(obrazWe + (y*(szerokosc)) + x + 1);

			pixWe[6] = *(obrazWe + (y*(szerokosc+1)) + x - 1);
			pixWe[7] = *(obrazWe + (y*(szerokosc+1)) + x);
			pixWe[8] = *(obrazWe + (y*(szerokosc+1)) + x + 1);

			//mnożenie przez obie macierze: wiersz obrazu * kolumna macierzy splotu
			pixWy[0] = ((pixWe[0] * -1) + (pixWe[1] * -2) + (pixWe[2] * 1))  + (pixWe[0] + (pixWe[0] * -1));
			pixWy[1] = ((pixWe[0] *  2) + (pixWe[2] * -2));
			pixWy[2] = (pixWe[0] + (pixWe[1] * 2)  + pixWe[2]) + (pixWe[0] + (pixWe[2] * -1)) ;

			pixWy[3] = ((pixWe[0] * -1) + (pixWe[1] * -2) + (pixWe[2] * 1));

			pixWy[6] = pixWe[0] * 1 + pixWe[2];
			pixWy[7] = pixWe[1] * 2 + pixWe[2];
			pixWy[8] = pixWe[2] * 1 + pixWe[2];
		}
	}
}



////////////////////////////////////////////////////////////////////////////////
// Liczy histogram z 7-bitowergo obrazu czarno-białego
// Parametry:
// [we] obraz* - wskaźnik na bufor[rozmiar] z obrazem czarno-białym
// [wy] hist - 129 elementowy, 8-bitowy histogram. Ostatni element jest dla liczb spoza zakresu
// [we] rozmiar - ilość pikseli do analizy
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void HistogramCB7(uint8_t *obraz, uint8_t *hist, uint32_t rozmiar)
{
	uint32_t histogram[129], temp;
	uint8_t pix;

	for (uint8_t n=0; n<129; n++)
		histogram[n] = 0;

	for (uint32_t n=0; n<rozmiar; n++)
	{
		pix = *(obraz+n);
		if (pix > 128)
			pix = 128;	//ostatnia pozycja zbiera sieci spoza zakresu 7-bit
		histogram[pix]++;
	}

	for (uint8_t n=0; n<129; n++)
	{
		temp = (uint8_t)(histogram[n] / DZIELNIK_HIST_CB);
		if (temp > 255)
			temp = 255;
		*(hist+n) = temp;
	}
}



////////////////////////////////////////////////////////////////////////////////
// Liczy histogram z obrazu RGB565
// Parametry:
// [we] obraz* - wskaźnik na bufor[2*rozmiar] z obrazem RGB565
// [wy] histR - 32 elementowy, 16-bitowy histogram
// [wy] histG - 64 elementowy, 16-bitowy histogram
// [wy] histB - 32 elementowy, 16-bitowy histogram
// [we] rozmiar - ilość pikseli do analizy
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void HistogramRGB565(uint8_t *obrazRGB565, uint8_t *histR, uint8_t *histG, uint8_t *histB, uint32_t rozmiar)
{
	uint8_t pix1, pix2;
	uint32_t histogramR[32], histogramG[64], histogramB[32], temp;

	for (uint8_t n=0; n<32; n++)
	{
		histogramR[n] = 0;
		histogramG[n] = 0;
		histogramG[n+32] = 0;
		histogramB[n] = 0;
	}

	for (uint32_t n=0; n<rozmiar; n++)
	{
		pix1 = *(obrazRGB565 + 2 * n + 0);		//B+G
		pix2 = *(obrazRGB565 + 2 * n + 1);		//G+R

		//bity czerwony i niebieski mają skalę 5-bitową (32), zielony 6-bitową (64).
		histogramR[pix1 >> 3]++;
		histogramG[(pix2 & 0x1F) + ((pix1 & 0x07) << 3)]++;
		histogramB[pix2 >> 5]++;
	}

	for (uint8_t n=0; n<32; n++)
	{
		temp = histogramR[n] / DZIELNIK_HIST_RGB;
		if (temp > 255)
			temp = 255;
		*(histR+n) = temp;

		temp = histogramB[n] / DZIELNIK_HIST_RGB;
		if (temp > 255)
			temp = 255;
		*(histB+n) = temp;
	}

	for (uint8_t n=0; n<64; n++)
	{
		temp = histogramG[n] / DZIELNIK_HIST_RGB;
		if (temp > 255)
			temp = 255;
		*(histG+n) = temp;
	}
}
