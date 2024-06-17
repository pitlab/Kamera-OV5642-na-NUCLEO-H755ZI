
//sterowanie wyświetlaczem
#define LCD_CS		1	//port B
#define LCD_WR		0	//port C
#define LCD_RS		3	//port C
#define LCD_RD		3	//port A
#define LCD_RST		2	//port C


struct _current_font
{
	unsigned char* font;
	unsigned char x_size;
	unsigned char y_size;
	unsigned char offset;
	unsigned char numchars;
} cfont;

unsigned char chOrient;		//orientacja wyświetlacza
unsigned char fch, fcl, bch, bcl;	//kolory czcionki i tła (bajt starszy i młodszy)
unsigned char _transparent;	//flaga określająca czy mamy rysować tło czy rysujemy na istniejącym

void LCD_init(void);
void LCD_Orient(unsigned char orient);
void LCD_data_write(unsigned char chData);
void LCD_data_read(unsigned char *chData);
void LCD_command_write(unsigned char chData);
void LCD_command_write_bezCS(unsigned char chData);
void LCD_WrBrightness(unsigned char chData);
void LCD_RdBrightness(unsigned char *chData);
void LCD_RdDisplayIdentification(void);
void LCD_RdDisplayStatus(void);
void LCD_write_com(unsigned char chData);
unsigned char LCD_RdCommand(unsigned char chIDcmd);
void delay(unsigned int nCzas);
void LCD_rect(unsigned short col, unsigned short row, unsigned short width, unsigned short height, unsigned short color) ;
void LCD_clear(void);
void Display_char(char znak);
void drawVLine(int x, int y, int len);
void drawHLine(int x, int y, int len);
void drawLine(int x1, int y1, int x2, int y2);
void setXY(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2);
void clrXY(void);
void setColorRGB(unsigned char r, unsigned char g, unsigned char b);
void setColor(unsigned short color);
unsigned short getColor(void);
void setBackColorRGB(unsigned char r, unsigned char g, unsigned char b);
void setBackColor(unsigned short color);
unsigned short getBackColor(void);
void setPixel(unsigned short color);
void drawPixel(int x, int y);
void drawRect(int x1, int y1, int x2, int y2);
void drawRoundRect(int x1, int y1, int x2, int y2);
void fillRect(int x1, int y1, int x2, int y2);
void fillRoundRect(int x1, int y1, int x2, int y2);
void printChar(unsigned char c, int x, int y);
unsigned char printChar2(unsigned char c, int x, int y);
void rotateChar(unsigned char c, int x, int y, int pos, int deg);
void setFont(unsigned char* font);
unsigned char GetFontX(void);
unsigned char GetFontY(void);
void print(char *st, int x, int y, int deg);
void print2(char *st, int x, int y);
void fillCircle(int x, int y, int radius);
void drawCircle(int x, int y, int radius);
void drawBitmap(int x, int y, int sx, int sy, const unsigned short* data);
void drawBitmapYUV(int x, int y, int sx, int sy, const unsigned short* data);
void YUV2RGB(unsigned short YUV1, unsigned short YUV2, unsigned char* RGB1L, unsigned char* RGB1H, unsigned char* RGB2L, unsigned char* RGB2H);
void YUV2RGB2(unsigned short YUV1, unsigned short YUV2, unsigned char* RGB1L, unsigned char* RGB1H, unsigned char* RGB2L, unsigned char* RGB2H);
