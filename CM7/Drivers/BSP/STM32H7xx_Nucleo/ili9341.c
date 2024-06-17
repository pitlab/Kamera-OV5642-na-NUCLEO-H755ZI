//////////////////////////////////////////////////////////////////////////////
//
// Moduł obsługi wyświetlacza TFT 320x240 dla płytki Nucleo-H755ZI-Q
//
// (c) PitLab 2019
// http://www.pitlab.pl
//////////////////////////////////////////////////////////////////////////////
#include "ili9341.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
#include "sys_def.h"
#include "display.h"
#include <string.h>


////////////////////////////////////////////////////////////////////////////////
// Wysy�a polecenie do wy�wietlacza LCD
// Parametry: chDane
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void LCD_write_com(unsigned char chData)
{
	GPIOB->BSRR = (1<<(LCD_CS+CLEAR));							//PB1=CS=0
	GPIOC->BSRR = (1<<(LCD_RS+CLEAR)) + (1<<(LCD_WR+CLEAR));	//PC3=RS=0
	GPIOA->BSRR = (1<<LCD_RD);									//PA3=RD=1

	//dane są rozsiane po różnych portach, więc poskładaj je
	GPIOA->BSRR = (1<<(8+ ((chData & 0x40) != 0x40)*CLEAR)); 	//D6 @ PA.8
	GPIOD->BSRR = (1<<(15+((chData & 0x02) != 0x02)*CLEAR)); 	//D1 @ PD.15
	GPIOE->BSRR = (1<<(11+((chData & 0x20) != 0x20)*CLEAR))|	//D5 @ PE.11
				  (1<<(13+((chData & 0x08) != 0x08)*CLEAR))|	//D3 @ PE.13
				  (1<<(14+((chData & 0x10) != 0x10)*CLEAR));	//D4 @ PE.14
	GPIOG->BSRR = (1<<(9+ ((chData & 0x01) != 0x01)*CLEAR))|	//D0 @ PG.9
				  (1<<(12+((chData & 0x80) != 0x80)*CLEAR))|	//D7 @ PG.12
				  (1<<(14+((chData & 0x04) != 0x04)*CLEAR));	//D2 @ PG.14

	GPIOC->BSRR = (1<<LCD_WR);				//PC0=WR=1
}



////////////////////////////////////////////////////////////////////////////////
// Wysyła dane do wyświetlacza LCD
// Parametry: chDane
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void LCD_write_dat(unsigned char chData)
{
	GPIOB->BSRR = (1<<(LCD_CS+CLEAR));					//PB1=CS=0
	GPIOC->BSRR = (1<<LCD_RS) + (1<<(LCD_WR+CLEAR));	//PC3=RS=1 + PC0=WR=0
	GPIOA->BSRR = (1<<LCD_RD);							//PA3=RD=1

	//dane są rozsiane po różnych portach, więc poskładaj je
	GPIOA->BSRR = (1<<(8+ ((chData & 0x40) != 0x40)*CLEAR)); 	//D6 @ PA.8
	GPIOD->BSRR = (1<<(15+((chData & 0x02) != 0x02)*CLEAR)); 	//D1 @ PD.15
	GPIOE->BSRR = (1<<(11+((chData & 0x20) != 0x20)*CLEAR))|	//D5 @ PE.11
				  (1<<(13+((chData & 0x08) != 0x08)*CLEAR))|	//D3 @ PE.13
				  (1<<(14+((chData & 0x10) != 0x10)*CLEAR));	//D4 @ PE.14
	GPIOG->BSRR = (1<<(9+ ((chData & 0x01) != 0x01)*CLEAR))|	//D0 @ PG.9
				  (1<<(12+((chData & 0x80) != 0x80)*CLEAR))|	//D7 @ PG.12
				  (1<<(14+((chData & 0x04) != 0x04)*CLEAR));	//D2 @ PG.14

	GPIOC->BSRR = (1<<LCD_WR);				//PC0=WR=1
}



////////////////////////////////////////////////////////////////////////////////
// Konfiguracja wy�wietlacza LCD
// Parametry: nic
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void LCD_init(void)
{
	HAL_Delay(20);
	// LCD_RESET 1 - 0 - 1
	GPIOC->BSRR = (1<<LCD_RST);			//PC2=RES=1
	HAL_Delay(10);
	GPIOC->BSRR = (1<<(LCD_RST+CLEAR));	//PC2=RES=0
	HAL_Delay(20);
	GPIOC->BSRR = (1<<LCD_RST);			//PC2=RES=1
	HAL_Delay(20);
	// CS HIGH, WR HIGH, RD HIGH, CS LOW
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	GPIOC->BSRR = (1<<LCD_WR);			//PC0=WR=1
	GPIOA->BSRR = (1<<LCD_RD);			//PA3=RD=1

	//dodane
	LCD_write_com(0xEF);
	LCD_write_dat(0x03);
	LCD_write_dat(0x80);
	LCD_write_dat(0x02);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0xCF);
	LCD_write_dat(0x00);
	LCD_write_dat(0xC1);
	LCD_write_dat(0x30);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0xED);
	LCD_write_dat(0x64);
	LCD_write_dat(0x03);
	LCD_write_dat(0x12);
	LCD_write_dat(0x81);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	//dodane
	LCD_write_com(0xE8);
	LCD_write_dat(0x85);
	LCD_write_dat(0x00);
	LCD_write_dat(0x79);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0xCB);
	LCD_write_dat(0x39);
	LCD_write_dat(0x2C);
	LCD_write_dat(0x00);
	LCD_write_dat(0x34);
	LCD_write_dat(0x02);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	//dodane
	LCD_write_com(0xF7);	//?
	LCD_write_dat(0x20);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0xEA);	//?
	LCD_write_dat(0x00);
	LCD_write_dat(0x00);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0xC0); //Power control
	LCD_write_dat(0x23); //VRH[5:0]
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0xC1); //Power control
	LCD_write_dat(0x11); //SAP[2:0];BT[3:0]
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0xC2);	//?
	LCD_write_dat(0x11);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0xC5); 	//VCM control 1
	LCD_write_dat(0x3E);
	LCD_write_dat(0x28);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0xC7);	//VCM control 2
	LCD_write_dat(0x86);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0x3A); //COLMOD: Pixel Format Set
	LCD_write_dat(0x55);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0x36); // Memory Access Control
	LCD_write_dat( (0<<7)|	//MY Row Address Order	- poziomo
				   (0<<6)|	//MX Column Address Order
				   (1<<5)|	//MV Row / Column Exchange
				   (1<<4)|	//ML Vertical Refresh Order
				   (1<<3)|	//BGR RGB-BGR Order
				   (0<<2));	//MH Horizontal Refresh ORDER
	/*LCD_write_dat( (1<<7)|	//MY Row Address Order	- poziomo
				   (1<<6)|	//MX Column Address Order
				   (1<<5)|	//MV Row / Column Exchange
				   (1<<4)|	//ML Vertical Refresh Order
				   (1<<3)|	//BGR RGB-BGR Order
				   (1<<2));	//MH Horizontal Refresh ORDER*/

	LCD_write_com(0xB1); // Frame Rate Control
	LCD_write_dat(0x00);
	LCD_write_dat(0x1B);		//70Hz
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0xB6); // Display Function Control
	LCD_write_dat(0x0A);
	LCD_write_dat(0x82);
	LCD_write_dat(0x27);
	LCD_write_dat(0x0F);	//sprawdzi� wziete na pa��
	GPIOB->BSRR = (1<<LCD_CS);			//PF3=CS=1

	LCD_write_com(0xF2); // 3Gamma Function Disable
	LCD_write_dat(0x00);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0xF7); // Pump ratio control
	LCD_write_dat(0x20);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0xF1);
	LCD_write_dat(0x01);
	LCD_write_dat(0x30);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0x26); //Gamma curve selected
	LCD_write_dat(0x01);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0xE0); //Set Gamma
	LCD_write_dat(0x0F);
	LCD_write_dat(0x3F);
	LCD_write_dat(0x2F);
	LCD_write_dat(0x0C);
	LCD_write_dat(0x10);
	LCD_write_dat(0x0A);
	LCD_write_dat(0x53);
	LCD_write_dat(0xD5);
	LCD_write_dat(0x40);
	LCD_write_dat(0x0A);
	LCD_write_dat(0x13);
	LCD_write_dat(0x03);
	LCD_write_dat(0x08);
	LCD_write_dat(0x03);
	LCD_write_dat(0x00);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0xE1); //Set Gamma
	LCD_write_dat(0x00);
	LCD_write_dat(0x00);
	LCD_write_dat(0x10);
	LCD_write_dat(0x03);
	LCD_write_dat(0x0F);
	LCD_write_dat(0x05);
	LCD_write_dat(0x2C);
	LCD_write_dat(0xA2);
	LCD_write_dat(0x3F);
	LCD_write_dat(0x05);
	LCD_write_dat(0x0E);
	LCD_write_dat(0x0C);
	LCD_write_dat(0x37);
	LCD_write_dat(0x3C);
	LCD_write_dat(0x0F);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	HAL_Delay(50);
	LCD_write_com(0x11); //Exit Sleep
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	HAL_Delay(120);
	LCD_write_com(0x29); //display on
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	HAL_Delay(50);

	setColor(GREEN);
	setBackColor(BLACK);
	chOrient = POZIOMO;
	LCD_clear();
}



////////////////////////////////////////////////////////////////////////////////
// ustawia orientacj� ekranu
// Parametry: orient - orientacja
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void LCD_Orient(unsigned char orient)
{
	LCD_write_com(0x36); // Memory Access Control
	if (orient)	//POZIOMO
	{
		LCD_write_dat((1<<2)|	//MH Horizontal Refresh ORDER
					  (1<<3)|	//BGR RGB-BGR Order: 0: RGB=RGB; 1:RGB=BGR
					  (1<<4)|	//ML Vertical Refresh Order
					  (1<<5)|	//MV Row / Column Exchange
					  (1<<6)|	//MX Column Address Order
					  (1<<7));	//MY Row Address Order
	}
	else	//PIONOWO
	{
		LCD_write_dat((0<<2)|	//MH Horizontal Refresh ORDER
					  (1<<3)|	//BGR RGB-BGR Order: 0: RGB=RGB; 1:RGB=BGR
					  (0<<4)|	//ML Vertical Refresh Order
					  (0<<5)|	//MV Row / Column Exchange
					  (1<<6)|	//MX Column Address Order
					  (1<<7));	//MY Row Address Order
	}
	chOrient = orient;
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
}



////////////////////////////////////////////////////////////////////////////////
// Rysuj prostok�t wype�niony kolorem
// Parametry: nCzas - op�nienie w ms
// Zwraca: nic
// Czas rysowania pe�nego ekranu: 174ms
////////////////////////////////////////////////////////////////////////////////
void LCD_rect(unsigned short col, unsigned short row, unsigned short width, unsigned short height, unsigned short color)
{
	int i,j;

	LCD_write_com(0x2a); // Column Address Set
	LCD_write_dat(row>>8);
	LCD_write_dat(row);
	LCD_write_dat((row+height-1)>>8);
	LCD_write_dat(row+height-1);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	LCD_write_com(0x2b); // Page Address Set
	LCD_write_dat(col>>8);
	LCD_write_dat(col);
	LCD_write_dat((col+width-1)>>8);
	LCD_write_dat(col+width-1);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	LCD_write_com(0x2c); // Memory Write
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	unsigned char chigh=color >> 8;
	unsigned char clow=color;

	for(i=0; i<width; i++)
	{
		for(j=0; j<height; j++)
		{
			LCD_write_dat(chigh);
			LCD_write_dat(clow);
		}
	}
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
}



////////////////////////////////////////////////////////////////////////////////
// zape�nij ca�y ekran kolorem
// Parametry: nCzas - op�nienie w ms
// Zwraca: nic
// Czas czyszczenia ekranu: 548,8ms
////////////////////////////////////////////////////////////////////////////////
void LCD_clear(void)
{
	unsigned short y;

	setColor(BLACK);
	setBackColor(BLACK);

	LCD_write_com(0x2a); // Column Address Set
	LCD_write_dat(0);
	LCD_write_dat(0);
	LCD_write_dat(1);
	LCD_write_dat(0x3F);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0x2b); // Page Address Set
	LCD_write_dat(0);
	LCD_write_dat(0);
	LCD_write_dat(0);
	LCD_write_dat(0xEF);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0x2c);	// Memory Write
	LCD_write_dat(bch);
	LCD_write_dat(bcl);

	for(y=0; y<38400; y++)		// 240*320/2
	{
		GPIOC->BSRR = (1<<(LCD_WR+CLEAR));	//PCO=WR=0
		GPIOC->BSRR = (1<<LCD_WR);			//PC0=WR=1
		GPIOC->BSRR = (1<<(LCD_WR+CLEAR));	//PCO=WR=0
		GPIOC->BSRR = (1<<LCD_WR);			//PC0=WR=1

		GPIOC->BSRR = (1<<(LCD_WR+CLEAR));	//PCO=WR=0
		GPIOC->BSRR = (1<<LCD_WR);			//PC0=WR=1
		GPIOC->BSRR = (1<<(LCD_WR+CLEAR));	//PCO=WR=0
		GPIOC->BSRR = (1<<LCD_WR);			//PC0=WR=1
	}
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
}



////////////////////////////////////////////////////////////////////////////////
// Rysuje poziom� lini�
// Parametry: x, y wsp�rz�dne pocz�tku
// len - d�ugo�c linii
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void drawHLine(int x, int y, int len)
{
	int i;

	if (len < 0)
	{
		len = -len;
		x -= len;
	}

	//GPIOB->BSRR = (1<<(LCD_CS+CLEAR));	// CS 0
	GPIOB->BSRR = (1<<(LCD_CS+CLEAR));	// CS 0
	setXY(x, y, x+len, y);

	for (i=0; i<len+1; i++)
	{
		LCD_write_dat(fch);
		LCD_write_dat(fcl);
	}

	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	clrXY();
}



////////////////////////////////////////////////////////////////////////////////
// Rysuje pionow� lini�
// Parametry: x, y wsp�rz�dne pocz�tku
// len - d�ugo�c linii
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void drawVLine(int x, int y, int len)
{
	int i;

	if (len < 0)
	{
		len = -len;
		y -= len;
	}
	GPIOB->BSRR = (1<<(LCD_CS+CLEAR));	// CS 0
	setXY(x, y, x, y+len);

	for (i=0; i<len+1; i++)
	{
		LCD_write_dat(fch);
		LCD_write_dat(fcl);
	}
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	clrXY();
}


////////////////////////////////////////////////////////////////////////////////
// ustawia parametry pami�ci do rysowania linii
// Parametry: x, y wsp�rz�dne pocz�tku
// len - d�ugo�c linii
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void setXY(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2)
{
	unsigned short sTemp;

	if (chOrient == PIONOWO)
	{
		sTemp = x1;
		x1 = y1;
		y1 = sTemp;
		sTemp = x2;
		x2 = y2;
		y2 = sTemp;
		y1=DISP_Y_SIZE - y1;
		y2=DISP_Y_SIZE - y2;
		sTemp = y1;
		y1 = y2;
		y2 = sTemp;
	}

	LCD_write_com(0x2A); //column
	LCD_write_dat(x1>>8);
	LCD_write_dat(x1);
	LCD_write_dat(x2>>8);
	LCD_write_dat(x2);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0x2B); //page
	LCD_write_dat(y1>>8);
	LCD_write_dat(y1);
	LCD_write_dat(y2>>8);
	LCD_write_dat(y2);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	LCD_write_com(0x2C); //write
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
}




////////////////////////////////////////////////////////////////////////////////
// zeruje parametry pami�ci do rysowania linii
// Parametry:nic
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void clrXY(void)
{
	if (chOrient == PIONOWO)
		setXY(0, 0, DISP_X_SIZE, DISP_Y_SIZE);
	else
		setXY(0, 0, DISP_Y_SIZE, DISP_X_SIZE);
}



////////////////////////////////////////////////////////////////////////////////
// Ustawia kolor rysowania jako RGB
// Parametry: r, g, b - sk�adowe RGB koloru
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void setColorRGB(unsigned char r, unsigned char g, unsigned char b)
{
	fch = ((r & 0xF8) | g>>5);
	fcl = ((g & 0x1C)<<3 | b>>3);
}



////////////////////////////////////////////////////////////////////////////////
// Ustawia kolor rysowania jako natywny dla wy�wietlacza 5R+6G+5B
// Parametry: color - kolor
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void setColor(unsigned short color)
{
	fch = (unsigned char)(color>>8);
	fcl = (unsigned char)(color & 0xFF);
}



////////////////////////////////////////////////////////////////////////////////
// pobiera aktywny kolor
// Parametry: nic
// Zwraca: kolor
////////////////////////////////////////////////////////////////////////////////
unsigned short getColor(void)
{
	return (fch<<8) | fcl;
}



////////////////////////////////////////////////////////////////////////////////
// Ustawia kolor t�a jako RGB
// Parametry: r, g, b - sk�adowe RGB koloru
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
//void setBackColorRGB(unsigned char r, unsigned char g, unsigned char b)
//{
//	bch=((r&248)|g>>5);
//	bcl=((g&28)<<3|b>>3);
//}



////////////////////////////////////////////////////////////////////////////////
// Ustawia kolor t�a jako natywny dla wy�wietlacza 5R+6G+5B
// Parametry: color - kolor
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void setBackColor(unsigned short color)
{
	if (color == TRANSPARENT)
		_transparent = 1;
	else
	{
		bch = (unsigned char)(color>>8);
		bcl = (unsigned char)(color & 0xFF);
		_transparent = 0;
	}
}



////////////////////////////////////////////////////////////////////////////////
// pobiera aktywny kolor t�a
// Parametry: nic
// Zwraca: kolor
////////////////////////////////////////////////////////////////////////////////
unsigned short getBackColor(void)
{
	return (bch<<8) | bcl;
}



////////////////////////////////////////////////////////////////////////////////
// zapisuje piksel do pami�ci bez machania hardwarem
// Parametry: color - kolor rrrrrggggggbbbbb
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void setPixel(unsigned short color)
{
	LCD_write_dat(color>>8);
	LCD_write_dat(color & 0xFF);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
}



////////////////////////////////////////////////////////////////////////////////
// rysuje piksel o wsp�prz�dnych x,y we wcze�niej zdefiniowanym kolorze
// Parametry: x, y - wsp�rz�dne
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void drawPixel(int x, int y)
{
	GPIOB->BSRR = (1<<(LCD_CS+CLEAR));	// CS 0
	setXY(x, y, x, y);
	setPixel((fch<<8)|fcl);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	clrXY();
}



////////////////////////////////////////////////////////////////////////////////
// rysuj lini� o wsp�prz�dnych x1,y1, x2,y3 we wcze�niej zdefiniowanym kolorze
// Parametry: x, y - wsp�rz�dne
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void drawLine(int x1, int y1, int x2, int y2)
{
	if (y1==y2)
		drawHLine(x1, y1, x2-x1);
	else if (x1==x2)
		drawVLine(x1, y1, y2-y1);
	else
	{
		unsigned int	dx = (x2 > x1 ? x2 - x1 : x1 - x2);
		short			xstep =  x2 > x1 ? 1 : -1;
		unsigned int	dy = (y2 > y1 ? y2 - y1 : y1 - y2);
		short			ystep =  y2 > y1 ? 1 : -1;
		int				col = x1, row = y1;

		GPIOB->BSRR = (1<<(LCD_CS+CLEAR));	// CS 0
		if (dx < dy)
		{
			int t = - (dy >> 1);
			while (1)
			{
				setXY (col, row, col, row);
				LCD_write_dat (fch);
				LCD_write_dat (fcl);
				GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
				if (row == y2)
					return;
				row += ystep;
				t += dx;
				if (t >= 0)
				{
					col += xstep;
					t   -= dy;
				}
			}
		}
		else
		{
			int t = - (dx >> 1);
			while (1)
			{
				setXY (col, row, col, row);
				LCD_write_dat (fch);
				LCD_write_dat (fcl);
				GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
				if (col == x2)
					return;
				col += xstep;
				t += dy;
				if (t >= 0)
				{
					row += ystep;
					t   -= dx;
				}
			}
		}
		GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	}
	clrXY();
}



////////////////////////////////////////////////////////////////////////////////
// rysuj prostok�t o wsp�prz�dnych x1,y1, x2,y2 we wcze�niej zdefiniowanym kolorze
// Parametry: x, y - wsp�rz�dne
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void drawRect(int x1, int y1, int x2, int y2)
{
	int nTemp;

	if (x1>x2)
	{
		nTemp = x1;
		x1 = x2;
		x2 = nTemp;
	}
	if (y1>y2)
	{
		nTemp = y1;
		y1 = y2;
		y2 = nTemp;
	}

	drawHLine(x1, y1, x2-x1);
	drawHLine(x1, y2, x2-x1);
	drawVLine(x1, y1, y2-y1);
	drawVLine(x2, y1, y2-y1);
}



////////////////////////////////////////////////////////////////////////////////
// rysuj zaokr�glony prostok�t o wsp�prz�dnych x1,y1, x2,y2 we wcze�niej zdefiniowanym kolorze
// Parametry: x, y - wsp�rz�dne
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void drawRoundRect(int x1, int y1, int x2, int y2)
{
	int nTemp;

	if (x1>x2)
	{
		nTemp = x1;
		x1 = x2;
		x2 = nTemp;
	}
	if (y1>y2)
	{
		nTemp = y1;
		y1 = y2;
		y2 = nTemp;
	}
	if ((x2-x1)>4 && (y2-y1)>4)
	{
		drawPixel(x1+1,y1+1);
		drawPixel(x2-1,y1+1);
		drawPixel(x1+1,y2-1);
		drawPixel(x2-1,y2-1);
		drawHLine(x1+2, y1, x2-x1-4);
		drawHLine(x1+2, y2, x2-x1-4);
		drawVLine(x1, y1+2, y2-y1-4);
		drawVLine(x2, y1+2, y2-y1-4);
	}
}


////////////////////////////////////////////////////////////////////////////////
// wype�nij kolorem prostok�t o wsp�prz�dnych x1, y1, x2, y2
// Parametry: x, y - wsp�rz�dne
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void fillRect(int x1, int y1, int x2, int y2)
{
	int i, nTemp;

	if (x1>x2)
	{
		nTemp = x1;
		x1 = x2;
		x2 = nTemp;
	}
	if (y1>y2)
	{
		nTemp = y1;
		y1 = y2;
		y2 = nTemp;
	}

	if (chOrient == PIONOWO)
	{
		for (i=0; i<((y2-y1)/2)+1; i++)
		{
			drawHLine(x1, y1+i, x2-x1);
			drawHLine(x1, y2-i, x2-x1);
		}
	}
	else
	{
		for (i=0; i<((x2-x1)/2)+1; i++)
		{
			drawVLine(x1+i, y1, y2-y1);
			drawVLine(x2-i, y1, y2-y1);
		}
	}

}



////////////////////////////////////////////////////////////////////////////////
// wype�nij kolorem zaokr�glony prostok�t o wsp�prz�dnych x1, y1, x2, y2
// Parametry: x, y - wsp�rz�dne
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void fillRoundRect(int x1, int y1, int x2, int y2)
{
	int i, nTemp;

	if (x1>x2)
	{
		nTemp = x1;
		x1 = x2;
		x2 = nTemp;
	}
	if (y1>y2)
	{
		nTemp = y1;
		y1 = y2;
		y2 = nTemp;
	}

	if ((x2-x1)>4 && (y2-y1)>4)
	{
		for (i=0; i<((y2-y1)/2)+1; i++)
		{
			switch(i)
			{
			case 0:
				drawHLine(x1+2, y1+i, x2-x1-4);
				drawHLine(x1+2, y2-i, x2-x1-4);
				break;
			case 1:
				drawHLine(x1+1, y1+i, x2-x1-2);
				drawHLine(x1+1, y2-i, x2-x1-2);
				break;
			default:
				drawHLine(x1, y1+i, x2-x1);
				drawHLine(x1, y2-i, x2-x1);
			}
		}
	}
}



////////////////////////////////////////////////////////////////////////////////
// pisze znak na miejscu o podanych wsp�rzednych
// Parametry: c - znak; x, y - wsp�rz�dne
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void printChar(unsigned char c, int x, int y)
{
	unsigned char i,ch;
	unsigned short j;
	unsigned short temp;
	int zz;

	GPIOB->BSRR = (1<<(LCD_CS+CLEAR));	// CS 0

	if (!_transparent)
	{
		if (chOrient == POZIOMO)
		{
			setXY(x,y,x+cfont.x_size-1,y+cfont.y_size-1);

			temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
			for(j=0;j<((cfont.x_size/8)*cfont.y_size);j++)
			{
				ch = cfont.font[temp];
				for(i=0;i<8;i++)
				{
					if((ch&(1<<(7-i)))!=0)
					{
						LCD_write_dat(fch);
						LCD_write_dat(fcl);
					}
					else
					{
						LCD_write_dat(bch);
						LCD_write_dat(bcl);
					}
				}
				temp++;
			}
			GPIOB->BSRR = (1<<LCD_CS);		//LCD_CS=1
		}
		else
		{
			temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;

			for(j=0;j<((cfont.x_size/8)*cfont.y_size);j+=(cfont.x_size/8))
			{
				setXY(x,y+(j/(cfont.x_size/8)),x+cfont.x_size-1,y+(j/(cfont.x_size/8)));
				for (zz=(cfont.x_size/8)-1; zz>=0; zz--)
				{
					ch=cfont.font[temp+zz];
					for(i=0;i<8;i++)
					{
						if((ch&(1<<i))!=0)
						{
							LCD_write_dat(fch);
							LCD_write_dat(fcl);
						}
						else
						{
							LCD_write_dat(bch);
							LCD_write_dat(bcl);
						}
					}
				}
				temp+=(cfont.x_size/8);
			}
			GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
		}
	}
	else
	{
		temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
		for(j=0;j<cfont.y_size;j++)
		{
			for (zz=0; zz<(cfont.x_size/8); zz++)
			{
				ch = cfont.font[temp+zz];
				for(i=0;i<8;i++)
				{
					if((ch&(1<<(7-i)))!=0)
					{
						setXY(x+i+(zz*8),y+j,x+i+(zz*8)+1,y+j+1);
						LCD_write_dat(fch);
						LCD_write_dat(fcl);
					}
				}
			}
			temp+=(cfont.x_size/8);
		}
	}

	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	clrXY();
}


////////////////////////////////////////////////////////////////////////////////
// pisze znak na miejscu o podanych wsp�rzednych
// funkcja odczytuje znaki kodowane przez GLCD Font Creator
// dane zapisywane s� kolumnami. Pierwszy znak czcionki oznacza szerko�� czcionki w kolumnach
// Parametry: c - znak; x, y - wsp�rz�dne
// Zwraca: szeroko�� znaku w pikselach
////////////////////////////////////////////////////////////////////////////////
unsigned char printChar2(unsigned char c, int x, int y)
{
	unsigned char i,ch, szer;
	unsigned short j;
	unsigned short start;
	int zz;

	GPIOB->BSRR = (1<<(LCD_CS+CLEAR));	// CS 0
	start = (c-cfont.offset)*((cfont.x_size*cfont.y_size/8)+1)+5;
	szer = cfont.font[(c-cfont.offset)*((cfont.x_size*cfont.y_size/8)+1)+4];

	for(j=0; j<szer*cfont.y_size/8; j+=(cfont.y_size/8))	//numer kolumny
	{
		setXY(x+(j/(cfont.y_size/8)), y, x+(j/(cfont.y_size/8)), y+cfont.y_size-1);	//obszar kolumny
		for (zz=0; zz<cfont.y_size/8; zz++)	//kolejne kom�rki kolumny
		{
			ch=cfont.font[start+zz];
			for(i=0; i<8; i++)
			{
				if((ch&(1<<i))!=0)
				{
					LCD_write_dat(fch);
					LCD_write_dat(fcl);
				}
				else
				{
					LCD_write_dat(bch);
					LCD_write_dat(bcl);
				}
			}
			GPIOB->BSRR = (1<<LCD_CS);		//LCD_CS=1
		}
		start += (cfont.y_size/8);
	}

	//dopisz za znakiem kilka pustych kolumn
	start = j+5*cfont.y_size/8;	//koniec pisania
	for(; j<start; j+=(cfont.y_size/8))	//numer kolumny
	{
		setXY(x+(j/(cfont.y_size/8)), y, x+(j/(cfont.y_size/8)), y+cfont.y_size-1);	//obszar kolumny
		for (zz=0; zz<cfont.y_size; zz++)	//kolejne kom�rki kolumny
		{
			LCD_write_dat(bch);
			LCD_write_dat(bcl);
		}
	}

	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	clrXY();
	return szer+5;
}



////////////////////////////////////////////////////////////////////////////////
// pisze znak na miejscu o podanych wsp�rzednych
// Parametry:
// *st - ci ag do wypisania
//  x, y - wsp�rz�dne
//  deg - k�t obrotu napisu
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void rotateChar(unsigned char c, int x, int y, int pos, int deg)
{
	unsigned char i,j,ch;
	unsigned short temp;
	int zz, newx, newy;
	double radian, sinrad, cosrad;

	//czasoch�onne operacje wykonaj przed p�tl�
	radian=deg*0.0175;
	sinrad = sin(radian);
	cosrad = cos(radian);

	GPIOB->BSRR = (1<<(LCD_CS+CLEAR));	// CS 0

	temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
	for(j=0;j<cfont.y_size;j++)
	{
		for (zz=0; zz<(cfont.x_size/8); zz++)
		{
			ch = cfont.font[temp+zz];
			for(i=0;i<8;i++)
			{
				newx = x+(((i+(zz*8)+(pos*cfont.x_size))*cosrad)-((j)*sinrad));
				newy = y+(((j)*cosrad)+((i+(zz*8)+(pos*cfont.x_size))*sinrad));

				setXY(newx,newy,newx+1,newy+1);

				if((ch&(1<<(7-i)))!=0)
				{
					//setPixel((fch<<8)|fcl);
					LCD_write_dat(fch);
					LCD_write_dat(fcl);
				}
				else
				{
					//if (!_transparent)
						//setPixel((bch<<8)|bcl);
						LCD_write_dat(bch);
						LCD_write_dat(bcl);
				}
				GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
			}
		}
		temp+=(cfont.x_size/8);
	}
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	clrXY();
}



////////////////////////////////////////////////////////////////////////////////
// ustawia aktualn� czcionk�
// Parametry: c - znak; x, y - wsp�rz�dne
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void setFont(unsigned char* font)
{
	cfont.font = font;
	cfont.x_size = *(font+0);
	cfont.y_size = *(font+1);
	cfont.offset = *(font+2);
	cfont.numchars = *(font+3);
}


unsigned char GetFontX(void)
{
	return cfont.x_size;
}

unsigned char GetFontY(void)
{
	return cfont.y_size;
}



////////////////////////////////////////////////////////////////////////////////
// pisze napis na miejscu o podanych wsp�rzednych
// Parametry:
// *st - ci ag do wypisania
//  x, y - wsp�rz�dne
//  deg - k�t obrotu napisu
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void print(char *st, int x, int y, int deg)
{
	int stl, i;

	stl = strlen(st);

	if (chOrient == PIONOWO)
	{
	if (x == RIGHT)
		x = (DISP_X_SIZE+1)-(stl*cfont.x_size);
	if (x == CENTER)
		x = ((DISP_X_SIZE+1)-(stl*cfont.x_size))/2;
	}
	else
	{
	if (x == RIGHT)
		x = (DISP_Y_SIZE+1)-(stl*cfont.x_size);
	if (x == CENTER)
		x = ((DISP_Y_SIZE+1)-(stl*cfont.x_size))/2;
	}

	for (i=0; i<stl; i++)
		if (deg == 0)
			printChar(*st++, x + (i*(cfont.x_size)), y);
		else
			rotateChar(*st++, x, y, i, deg);
}



////////////////////////////////////////////////////////////////////////////////
// pisze napis na miejscu o podanych wsp�rzednych
// Parametry:
// *st - ci ag do wypisania
//  x, y - wsp�rz�dne
//  deg - k�t obrotu napisu
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void print2(char *st, int x, int y)
{
	int stl, i, nFontSzer, nCharPos;

	stl = strlen(st);

	if (chOrient == PIONOWO)
	{
	if (x == RIGHT)
		x = (DISP_X_SIZE+1)-(stl*cfont.x_size);
	if (x == CENTER)
		x = ((DISP_X_SIZE+1)-(stl*cfont.x_size))/2;
	}
	else
	{
	if (x == RIGHT)
		x = (DISP_Y_SIZE+1)-(stl*cfont.x_size);
	if (x == CENTER)
		x = ((DISP_Y_SIZE+1)-(stl*cfont.x_size))/2;
	}

	nCharPos = 0;
	for (i=0; i<stl; i++)
	{
		nFontSzer = printChar2(*st++, x+nCharPos, y);
		nCharPos += nFontSzer;
	}
}



////////////////////////////////////////////////////////////////////////////////
// rysyje ko�o
// Parametry:
//  x, y - wsp�rz�dne
//  radius - promie�
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void drawCircle(int x, int y, int radius)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;

	//cbi(P_CS, B_CS);
	setXY(x, y + radius, x, y + radius);
	LCD_write_dat(fch);
	LCD_write_dat(fcl);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	setXY(x, y - radius, x, y - radius);
	LCD_write_dat(fch);
	LCD_write_dat(fcl);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	setXY(x + radius, y, x + radius, y);
	LCD_write_dat(fch);
	LCD_write_dat(fcl);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	setXY(x - radius, y, x - radius, y);
	LCD_write_dat(fch);
	LCD_write_dat(fcl);
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1

	while(x1 < y1)
	{
		if(f >= 0)
		{
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;
		setXY(x + x1, y + y1, x + x1, y + y1);
		LCD_write_dat(fch);
		LCD_write_dat(fcl);
		GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
		setXY(x - x1, y + y1, x - x1, y + y1);
		LCD_write_dat(fch);
		LCD_write_dat(fcl);
		GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
		setXY(x + x1, y - y1, x + x1, y - y1);
		LCD_write_dat(fch);
		LCD_write_dat(fcl);
		GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
		setXY(x - x1, y - y1, x - x1, y - y1);
		LCD_write_dat(fch);
		LCD_write_dat(fcl);
		GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
		setXY(x + y1, y + x1, x + y1, y + x1);
		LCD_write_dat(fch);
		LCD_write_dat(fcl);
		GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
		setXY(x - y1, y + x1, x - y1, y + x1);
		LCD_write_dat(fch);
		LCD_write_dat(fcl);
		GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
		setXY(x + y1, y - x1, x + y1, y - x1);
		LCD_write_dat(fch);
		LCD_write_dat(fcl);
		GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
		setXY(x - y1, y - x1, x - y1, y - x1);
		LCD_write_dat(fch);
		LCD_write_dat(fcl);
		GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	}
	//sbi(P_CS, B_CS);
	clrXY();
}



void fillCircle(int x, int y, int radius)
{
	int y1, x1;

	for(y1=-radius; y1<=0; y1++)
		for(x1=-radius; x1<=0; x1++)
			if(x1*x1+y1*y1 <= radius*radius)
			{
				drawHLine(x+x1, y+y1, 2*(-x1));
				drawHLine(x+x1, y-y1, 2*(-x1));
				break;
			}
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
}



////////////////////////////////////////////////////////////////////////////////
// wy�wietla bitmap�
// Parametry:
//  x, y - wsp�rz�dne ekranu
//  sx, sy - rozmiar bitmapy
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void drawBitmap(int x, int y, int sx, int sy, const unsigned short* data)
{
	unsigned short col;
	int tx, ty, tc;

	GPIOB->BSRR = (1<<(LCD_CS+CLEAR));	// CS 0
	if (chOrient == PIONOWO)
	{
		setXY(x, y, x+sx-1, y+sy-1);
		for (tc=0; tc<(sx*sy); tc++)
		{
			col = data[tc];
			LCD_write_dat(col>>8);
			LCD_write_dat(col & 0xff);
		}
		GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	}
	else
	{
		for (ty=0; ty<sy; ty++)
		{
			setXY(x, y+ty, x+sx-1, y+ty);
			for (tx=sx-1; tx>=0; tx--)
			{
				col = data[(ty*sx)+tx];
				LCD_write_dat(col>>8);
				LCD_write_dat(col & 0xff);
			}
			GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
		}
	}
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	clrXY();
}


void drawBitmapYUV(int x, int y, int sx, int sy, const unsigned short* data)
{
	int tx, ty, tc;
	unsigned char RGB1L, RGB1H, RGB2L, RGB2H;
	unsigned short YUV1, YUV2;

	GPIOB->BSRR = (1<<(LCD_CS+CLEAR));	// CS 0
	if (chOrient == PIONOWO)
	{
		setXY(x, y, x+sx-1, y+sy-1);
		for (tc=0; tc<(sx*sy/2); tc++)
		{
			YUV1 = data[tc*2+0];
			YUV2 = data[tc*2+1];
			YUV2RGB(YUV1, YUV2, &RGB1L,&RGB1H, &RGB2L, &RGB2H);
			LCD_write_dat(RGB1H);
			LCD_write_dat(RGB1L);
			LCD_write_dat(RGB2H);
			LCD_write_dat(RGB2L);
		}
		GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	}
	else
	{
		for (ty=0; ty<sy; ty++)
		{
			setXY(x, y+ty, x+sx-1, y+ty);
			for (tx=sx/2-1; tx>=0; tx--)
			{

				YUV1 = data[(ty*sx) + tx*2-0];
				YUV2 = data[(ty*sx) + tx*2-1];
				YUV2RGB(YUV1, YUV2, &RGB1L,&RGB1H, &RGB2L, &RGB2H);
				LCD_write_dat(RGB1H);
				LCD_write_dat(RGB1L);
				LCD_write_dat(RGB2H);
				LCD_write_dat(RGB2L);

				//col = data[(ty*sx)+tx];
				//LCD_write_dat(col & 0xff);
				//LCD_write_dat(col>>8);
			}
			GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
		}
	}
	GPIOB->BSRR = (1<<LCD_CS);			//PB1=CS=1
	clrXY();
}



////////////////////////////////////////////////////////////////////////////////
// wykonuje konwersję zkodowania YUV 4:2:2 kamery na RGB wyświetlacza
// Parametry:
// YUV1, YUV2 - słowa wejsciowe z kamery
// *RGB1L, *RGB1H, *RGB2L, *RGB2H - wskaźniki na bajty wyjściowe RGB do wyświetlacza
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void YUV2RGB(unsigned short YUV1, unsigned short YUV2, unsigned char* RGB1L, unsigned char* RGB1H, unsigned char* RGB2L, unsigned char* RGB2H)
{
	//ukłąd wejściowy: Y1, Cb(U), Y2, Cr(V)
	//konwersja
	//V = 0,877 * (R - Y) 	=> 	R = (V + 0,877 * Y) / 0,877
	//U = 0,492 * (B - Y) 	=> 	B = (U + 0,492 * Y) / 0,492
	//Y = 0,299 * R + 0,587 * G + 0,114 * B 	=>	G = (Y - (0,299 * R + 0,114 * B)) / 0,587


	uint8_t pixel1[3], pixel2[3];
	uint8_t  Y1, U, Y2, V;

	Y1 = (YUV1 & 0x00FF);
	U  = (YUV1 & 0xFF00) >> 8;
	Y2 = (YUV2 & 0x00FF);
	V  = (YUV2 & 0xFF00) >> 8;

	pixel1[COL_R] = (uint8_t)((V + 0.877 * Y1) / 1.877);
	pixel2[COL_R] = (uint8_t)((V + 0.877 * Y2) / 1.877);
	pixel1[COL_B] = (uint8_t)((U + 0.492 * Y1) / 1.492);
	pixel2[COL_B] = (uint8_t)((U + 0.492 * Y2) / 1.492);
	pixel1[COL_G] = (uint8_t)((Y1 - (0.299 * pixel1[COL_R] + 0.114 * pixel1[COL_B])) / 0.587);
	pixel2[COL_G] = (uint8_t)((Y2 - (0.299 * pixel2[COL_R] + 0.114 * pixel2[COL_B])) / 0.587);

	//formatowanie danych wyjściowych
	//fcl = ((g & 0x1C)<<3 | b>>3);
	//fch = ((r & 0xF8) | g>>5);
	*RGB1L = (pixel1[COL_G] & 0x1C) << 3 | pixel1[COL_B] >> 3;
	*RGB1H = ((pixel1[COL_R] & 0xF8) | (pixel1[COL_G] >> 5));
	*RGB2L = (pixel2[COL_G] & 0x1C) << 3 | pixel2[COL_B] >> 3;
	*RGB2H = ((pixel2[COL_R] & 0xF8) | (pixel2[COL_G] >> 5));
}


//ta wersja  jest OK obliczeniowo
void YUV2RGB2(unsigned short YUV1, unsigned short YUV2, unsigned char* RGB1L, unsigned char* RGB1H, unsigned char* RGB2L, unsigned char* RGB2H)
{
	uint8_t pixel1[3], pixel2[3];
	uint8_t  Y1, U, Y2, V;

	Y1 = (YUV1 & 0x00FF);
	U  = (YUV1 & 0xFF00) >> 8;
	Y2 = (YUV2 & 0x00FF);
	V  = (YUV2 & 0xFF00) >> 8;

	pixel1[COL_R] = (Y1 + (351 * (V - 128))) >> 8;
	pixel2[COL_R] = (Y2 + (351 * (V - 128))) >> 8;
	pixel1[COL_G] = (Y1 - (179 * (V - 128) + 86 * (U - 128))) >> 8;
	pixel2[COL_G] = (Y2 - (179 * (V - 128) + 86 * (U - 128))) >> 8;
	pixel1[COL_B] = (Y1 + (443 * (U - 128))) >> 8;
	pixel2[COL_B] = (Y2 + (443 * (U - 128))) >> 8;


//	R = Y + (351*(Cr – 128)) >> 8
//	G = Y – (179*(Cr – 128) + 86*(Cb – 128))>>8
//	B = Y + (443*(Cb – 128)) >> 8

	//formatowanie danych wyjściowych
	*RGB1L = (pixel1[COL_G] & 0x1C) << 3 | pixel1[COL_B] >> 3;
	*RGB1H = ((pixel1[COL_R] & 0xF8) | (pixel1[COL_G] >> 5));
	*RGB2L = (pixel2[COL_G] & 0x1C) << 3 | pixel2[COL_B] >> 3;
	*RGB2H = ((pixel2[COL_R] & 0xF8) | (pixel2[COL_G] >> 5));
}


