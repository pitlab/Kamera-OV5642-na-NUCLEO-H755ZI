//////////////////////////////////////////////////////////////////////////////
//
// AutoPitLot v3.0
// Moduł obsługi kamery
//
// (c) PitLab 2024
// http://www.pitlab.pl
//////////////////////////////////////////////////////////////////////////////
// KAMERA		Port uC				NUCLEO		PRZEWÓD
//  1 VCC		---					CN11-12		czerwony
//  2 GND		---					CN11-8		czarny
//  3 SCL		PF14, I2C4_SCL		CN12-50		żółty
//  4 SDA		PF15, I2C4_SDA		CN12-60		zielony
//  5 VSYNC		PB7, DCMI_VSYNC		CN11-21		brązowy
//  6 HREF		PA4, DCMI_HREF		CN11-32		niebieski
//  7 PCLK		PA6, DCMI_PIXCLK	CN12-13		niebieski
//  8 XCLK		PF9, TIM14_CH1		CN11-56		pomarańczowy
//  9 DOUT9 	PE6, DCMI_D7		CN11-62		biały
// 10 DOUT8		PE5, DCMI_D6		CN11-50		szary
// 11 DOUT7		PD3, DCMI_D5		CN11-40		fioletowy
// 12 DOUT6		PE4, DCMI_D4		CN11-48		niebieski
// 13 DOUT5		PC9, DCMI_D3		CN12-1		zielony
// 14 DOUT4 	PC8, DCMI_D2		CN12-2		pomarańczowy
// 15 DOUT3		PC7, DCMI_D1		CN12-19		żółty
// 16 DOUT2		PC6, DCMI_DO		CN12-4		czerwony
// 17 PWDN		PF11				CN12-62		czarny
// 18 RSV		---
// 19 DOUT1		---
// 20 DOUT0		---
//

//Zegar taktujący kamerę XCLK to 120MHz / 6 = 20MHz. Z zegarem 120/5 = 24MHz kamera podpięta na kabelkach nie chce już pracować
//Wypełnienie zegara (PWM):
// 2/6 - obraz stabilny lub mocno płynie w prawo
// 3/6 - obraz stabilny lub lekko płynie w lewo
// 4/6 - brak synchronizacji
//////////////////////////////////////////////////////////////////////////////
#include "kamera.h"
#include "stm32h7xx_nucleo.h"
#include "..\..\..\Common\Inc\errcode.h"
#include "..\..\..\..\include\PoleceniaKomunikacyjne.h"


////////////////////////////////////////////////////////////////////////////////
// Inicjalizacja pracy kamery
// Parametry: brak
// Zwraca: kod błędu HAL
////////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef KameraInit(void)
{
	HAL_StatusTypeDef err = 0;

	__HAL_RCC_DCMI_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	//zegar timera taktowany jest z APB1 = 120MHz, prescaler = 0
	//kamera wymaga zegara 24MHz (6-27MHz), więc zegar trzeba podzielić na 5
	TIM14->CR1 |= TIM_CR1_CEN;		//włącz timer taktujacy kamerę
	TIM14->ARR = KAMERA_DZIELNIK_ZEGARA - 1;		//częstotliwość PWM
	TIM14->CCR1 = KAMERA_WYPELN_PWM;			//wypełnienie PWM
	TIM14->CCER |= TIM_CCER_CC1E;	//włącz wyjście timera

	KameraPWDN(GPIO_PIN_SET);		//włącz PWND
	HAL_Delay(10);	//power on period
	KameraPWDN(GPIO_PIN_RESET);		//wyłącz PWND
	HAL_Delay(30);

	err = SprawdzKamere();		//sprawdź czy mamy kontakt z kamerą
	if (err)
		return err;

	Wyslij_I2C_Kamera(0x3103, 0x93);	//PLL clock select: [1] PLL input clock: 1=from pre-divider
	Wyslij_I2C_Kamera(0x3008, 0x82);	//system control 00: [7] software reset mode, [6] software power down mode {def=0x02}
	HAL_Delay(30);
	/*err = Wyslij_Blok_Kamera(ov5642_dvp_fmt_global_init);		//174ms @ 20MHz
	if (err)
		return err;*/
	err = Wyslij_Blok_Kamera(OV5642_RGB_QVGA);					//150ms @ 20MHz
	if (err)
		return err;

	//ustaw domyślne parametry pracy kamery
	KonfKam.sSzerWe = 1280;
	KonfKam.sWysWe = 960;
	KonfKam.sSzerWy = 320;
	KonfKam.sWysWy = 240;
	KonfKam.chTrybDiagn = 0;	//brak trybu diagnostycznego
	KonfKam.chFlagi = 0;

	err = UstawKamere(&KonfKam);
	if (err)
		return err;

	Wyslij_I2C_Kamera(0x4300, 0x6F);	//format control [7..4] 6=RGB656, [3..0] 1={R[4:0], G[5:3]},{G[2:0}, B[4:0]}

	return RozpocznijPraceDCMI(KonfKam.chFlagi & FUK1_ZDJ_FILM);	//1 = zdjecie, 0 = film (tylko ten jeden bit)
}



////////////////////////////////////////////////////////////////////////////////
// konfiguruje wybrane parametry kamery
// Parametry: konf - struktura konfiguracji kamery
// Zwraca: kod błędu HAL
////////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef UstawKamere(typKonfKam *konf)
{
	//uint8_t chReg;

	Wyslij_I2C_Kamera(0x5001, 0x7F);	//ISP control 01: [7] Special digital effects, [6] UV adjust enable, [5]1=Vertical scaling enable, [4]1=Horizontal scaling enable, [3] Line stretch enable, [2] UV average enable, [1] color matrix enable, [0] auto white balance AWB

	//ustaw rozdzielczość wejściową
	Wyslij_I2C_Kamera(0x3804, (uint8_t)(konf->sSzerWe>>8));		//Timing HW: [3:0] Horizontal width high byte 0x500=1280,  0x280=640, 0x140=320 (scale input}
	Wyslij_I2C_Kamera(0x3805, (uint8_t)(konf->sSzerWe & 0xFF));	//Timing HW: [7:0] Horizontal width low byte
	Wyslij_I2C_Kamera(0x3806, (uint8_t)(konf->sWysWe>>8));			//Timing VH: [3:0] HREF vertical height high byte 0x3C0=960, 0x1E0=480, 0x0F0=240
	Wyslij_I2C_Kamera(0x3807, (uint8_t)(konf->sWysWe & 0xFF));		//Timing VH: [7:0] HREF vertical height low byte

	//ustaw rozdzielczość wyjściową
	Wyslij_I2C_Kamera(0x3808, (uint8_t)(konf->sSzerWy>>8));		//Timing DVPHO: [3:0] output horizontal width high byte [11:8]
	Wyslij_I2C_Kamera(0x3809, (uint8_t)(konf->sSzerWy & 0xFF));	//Timing DVPHO: [7:0] output horizontal width low byte [7:0]
	Wyslij_I2C_Kamera(0x380a, (uint8_t)(konf->sWysWy>>8));			//Timing DVPVO: [3:0] output vertical height high byte [11:8]
	Wyslij_I2C_Kamera(0x380b, (uint8_t)(konf->sWysWy & 0xFF));		//Timing DVPVO: [7:0] output vertical height low byte [7:0]

	//wzór testowy
	switch(konf->chTrybDiagn)
	{
	case TDK_KRATA_CB:	//czarnobiała karata
		Wyslij_I2C_Kamera(0x503d , 0x85);	//test pattern: B/W square
		Wyslij_I2C_Kamera(0x503e, 0x1a);	//PRE ISP TEST SETTING2 [7] reserved, [6:4] 1=random data pattern seed enable, [3] 1=test pattern square b/w mode, [2] 1=add test pattern on image data, [1:0] 0=color bar, 1=random data, 2=square data, 3=black image
		break;

	case TDK_PASKI:		//7 pionowych pasków
		Wyslij_I2C_Kamera(0x503d , 0x80);	//test pattern: color bar
		Wyslij_I2C_Kamera(0x503e, 0x00);	//PRE ISP TEST SETTING2 [7] reserved, [6:4] 1=random data pattern seed enable, [3] 1=test pattern square b/w mode, [2] 1=add test pattern on image data, [1:0] 0=color bar, 1=random data, 2=square data, 3=black image
		break;

	case TDK_PRACA:		//normalna praca
	default:
	}

	return 0;
	//ustaw rotację w poziomie i pionie
	//chReg = 0x80 + ((konf->chFlagi && FUK1_OBR_PION) << 6) +  ((konf->chFlagi && FUK1_OBR_POZ) << 5);
	//return Wyslij_I2C_Kamera(0x3818, chReg);	//TIMING TC REG18: [6] mirror, [5] Vertial flip, [4] 1=thumbnail mode,  [3] 1=compression, [1] vertical subsample 1/4, [0] vertical subsample 1/2  <def:0x80>
	//for the mirror function it is necessary to set registers 0x3621 [5:4] and 0x3801
}



////////////////////////////////////////////////////////////////////////////////
// uruchamia DCMI w trybie pojedyńczego zdjęcia jako aparat lub ciagłej pracy jako kamera
// Parametry: chAparat - 1 = tryb pojedyńczego zdjęcia, 0 = tryb filmu
// Zwraca: kod błędu HAL
////////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef RozpocznijPraceDCMI(uint8_t chAparat)
{
	HAL_StatusTypeDef err;

	hdcmi.Instance = DCMI;
	hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
	hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_FALLING;
	hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_LOW;
	hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_HIGH;
	hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
	hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
	hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
	hdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
	hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
	hdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
	hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;
	hdcmi.Instance->IER = DCMI_IT_FRAME | DCMI_IT_OVR | DCMI_IT_ERR | DCMI_IT_VSYNC | DCMI_IT_LINE;
	err = HAL_DCMI_Init(&hdcmi);
	if (err)
		return err;

	//konfiguracja DMA do DCMI
	hdma_dcmi.Instance = DMA1_Stream0;
	hdma_dcmi.Init.Request = DMA_REQUEST_DCMI;
	hdma_dcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_dcmi.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_dcmi.Init.MemInc = DMA_MINC_ENABLE;
	hdma_dcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	hdma_dcmi.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	if (chAparat)		//1 = zdjecie, 0 = film
		hdma_dcmi.Init.Mode = DMA_NORMAL;
	else
		hdma_dcmi.Init.Mode = DMA_CIRCULAR;
	hdma_dcmi.Init.Priority = DMA_PRIORITY_HIGH;
	hdma_dcmi.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	err = HAL_DMA_Init(&hdma_dcmi);
	if (err)
		return err;

	//Konfiguracja transferu DMA z DCMI do pamięci
	if (chAparat)		//1 = zdjecie, 0 = film
		return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)nBuforKamery, ROZM_BUF32_KAM);
	else
		return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)nBuforKamery, ROZM_BUF32_KAM);
}



HAL_StatusTypeDef InitKamera1(void)
{
	HAL_StatusTypeDef err;

	TIM14->CR1 |= TIM_CR1_CEN;		//włącz timer taktujacy kamerę
	TIM14->ARR = KAMERA_DZIELNIK_ZEGARA - 1;	//częstotliwość PWM
	TIM14->CCR1 = KAMERA_WYPELN_PWM;			//wypełnienie PWM
	TIM14->CCER |= TIM_CCER_CC1E;	//włącz wyjście timera

	Wyslij_I2C_Kamera(0x3103, 0x93);	//PLL clock select: [1] PLL input clock: 1=from pre-divider
	Wyslij_I2C_Kamera(0x3008, 0x82);	//system control 00: [7] software reset mode, [6] software power down mode {def=0x02}
	HAL_Delay(100);
	//err = Wyslij_Blok_Kamera(ov5642_dvp_fmt_global_init);
	err = Wyslij_Blok_Kamera(OV5642_RGB_QVGA);	//150ms
	if (err)
		return err;
	//Wyslij_I2C_Kamera(0x4300, 0x6F);
	//Wyslij_I2C_Kamera(0x503e, 0x00);	//pre ISP test settings 2: [6:4]1=random data pattern seed enable, [3] test pattern square black/white enabled, [2] add test pattern on image data, [1:0] 0=color bar, 1=random data, 2=square data, 3=black image
	//Wyslij_I2C_Kamera(0x4741, 0x07);	//test pattern [2] test pattern enable, [1] pattern 0/1, [0] 0=pattern 10-bit, 1=8-bit

	Wyslij_I2C_Kamera(0x503d , 0x80);	//test pattern: color bar
	Wyslij_I2C_Kamera(0x503e, 0x00);
	Wyslij_I2C_Kamera(0x4300, 0x60);	//format control [7..4] 6=RGB656, [3..0] 1={R[4:0], G[5:3]},{G[2:0}, B[4:0]}


	//HAL_DCMI_ConfigCrop(&hdcmi, (2592-640)/2, (1944-480)/2, 640-1, 480-1);
	//HAL_DCMI_EnableCrop(&hdcmi);

	//Konfiguracja transferu DMA z DCMI do pamięci
	return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)nBuforKamery, ROZM_BUF32_KAM);
	/*err = HAL_DCMI_Stop(&hdcmi);
	if (err)
		return err;
	return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)nBuforKamery, ROZM_BUF32_KAM);*/
}



HAL_StatusTypeDef InitKamera2(void)
{
	HAL_StatusTypeDef err;

	TIM14->CR1 |= TIM_CR1_CEN;		//włącz timer taktujacy kamerę
	TIM14->ARR = KAMERA_DZIELNIK_ZEGARA - 1;	//częstotliwość PWM
	TIM14->CCR1 = KAMERA_WYPELN_PWM;			//wypełnienie PWM
	TIM14->CCER |= TIM_CCER_CC1E;	//włącz wyjście timera

	Wyslij_I2C_Kamera(0x3103, 0x93);	//PLL clock select: [1] PLL input clock: 1=from pre-divider
	Wyslij_I2C_Kamera(0x3008, 0x82);	//system control 00: [7] software reset mode, [6] software power down mode {def=0x02}
	HAL_Delay(100);
	//err = Wyslij_Blok_Kamera(ov5642_dvp_fmt_global_init);
	err = Wyslij_Blok_Kamera(OV5642_RGB_QVGA);
	if (err)
		return err;
	//Wyslij_Blok_Kamera(OV5642_RGB_QVGA);					//150ms
	//Wyslij_I2C_Kamera(0x4300, 0x61);
	//Wyslij_I2C_Kamera(0x4745, 0x00);	//data order [1:0]: 0=D[9:0], 1=D[7:0],D[9:8], 2=D[1:0],D[9:2]
	//Wyslij_I2C_Kamera(0x4741, 0x07);	//test pattern [2] test pattern enable, [1] pattern 0/1, [0] 0=pattern 10-bit, 1=8-bit

	//Wyslij_I2C_Kamera(0x503d , 0x85);	//test pattern: color square
	//Wyslij_I2C_Kamera(0x503e, 0x12);

	Wyslij_I2C_Kamera(0x503d , 0x80);	//test pattern: color bar
	Wyslij_I2C_Kamera(0x503e, 0x00);
	Wyslij_I2C_Kamera(0x4300, 0x61);	//format control [7..4] 6=RGB656, [3..0] 1={R[4:0], G[5:3]},{G[2:0}, B[4:0]}

	//HAL_DCMI_ConfigCrop(&hdcmi, (2592-640)/2, (1944-480)/2, 640-1, 480-1);
	//HAL_DCMI_EnableCrop(&hdcmi);

	//Konfiguracja transferu DMA z DCMI do pamięci
	return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)nBuforKamery, ROZM_BUF32_KAM);
	/*err = HAL_DCMI_Stop(&hdcmi);
	if (err)
		return err;
	return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)nBuforKamery, ROZM_BUF32_KAM);*/
}



HAL_StatusTypeDef InitKamera3(void)
{
	HAL_StatusTypeDef err;

	TIM14->CR1 |= TIM_CR1_CEN;		//włącz timer taktujacy kamerę
	TIM14->ARR = KAMERA_DZIELNIK_ZEGARA - 1;	//częstotliwość PWM
	TIM14->CCR1 = KAMERA_WYPELN_PWM;			//wypełnienie PWM
	TIM14->CCER |= TIM_CCER_CC1E;	//włącz wyjście timera

	Wyslij_I2C_Kamera(0x3103, 0x93);	//PLL clock select: [1] PLL input clock: 1=from pre-divider
	Wyslij_I2C_Kamera(0x3008, 0x82);	//system control 00: [7] software reset mode, [6] software power down mode {def=0x02}
	HAL_Delay(100);
	//err = Wyslij_Blok_Kamera(ov5642_dvp_fmt_global_init);
	//if (err)
		//return err;
	err = Wyslij_Blok_Kamera(OV5642_RGB_QVGA);					//150ms
	if (err)
		return err;
	//Wyslij_I2C_Kamera(0x4300, 0x62);
	//Wyslij_I2C_Kamera(0x4745, 0x01);	//data order [1:0]: 0=D[9:0], 1=D[7:0],D[9:8], 2=D[1:0],D[9:2]
	//Wyslij_I2C_Kamera(0x4741, 0x07);	//test pattern [2] test pattern enable, [1] pattern 0/1, [0] 0=pattern 10-bit, 1=8-bit
	Wyslij_I2C_Kamera(0x503d , 0x80);	//test pattern: color bar
	Wyslij_I2C_Kamera(0x503e, 0x00);
	Wyslij_I2C_Kamera(0x4300, 0x62);	//format control [7..4] 6=RGB656, [3..0] 1={R[4:0], G[5:3]},{G[2:0}, B[4:0]}


	//HAL_DCMI_ConfigCrop(&hdcmi, (2592-640)/2, (1944-480)/2, 640-1, 480-1);
	//HAL_DCMI_EnableCrop(&hdcmi);

	//Konfiguracja transferu DMA z DCMI do pamięci
	return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)nBuforKamery, ROZM_BUF32_KAM);
	/*err = HAL_DCMI_Stop(&hdcmi);
	if (err)
		return err;
	return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)nBuforKamery, ROZM_BUF32_KAM);*/
}



HAL_StatusTypeDef InitKamera4(void)
{
	HAL_StatusTypeDef err;

	TIM14->CR1 = (0 >> TIM_CR1_CKD_Pos) |	//CKD[1:0] bits (clock division) - zegar wejsciowy bez dzielnika
					TIM_CR1_UDIS |			//<Update disable = 1, nie generuj przerwania ani eventu, zawartość rejestrów ARR, PSC i CCR1 bez zmian
					TIM_CR1_CEN;			//Counter enable = 1, włącz timer taktujacy kamerę
	TIM14->ARR = KAMERA_DZIELNIK_ZEGARA - 1;		//częstotliwość PWM
	TIM14->CCR1 = KAMERA_WYPELN_PWM;			//wypełnienie PWM
	TIM14->CCER |= TIM_CCER_CC1E;	//włącz wyjście timera

	Wyslij_I2C_Kamera(0x3103, 0x93);	//PLL clock select: [1] PLL input clock: 1=from pre-divider
	Wyslij_I2C_Kamera(0x3008, 0x82);	//system control 00: [7] software reset mode, [6] software power down mode {def=0x02}
	HAL_Delay(100);
	//Wyslij_Blok_Kamera(ov5642_dvp_fmt_global_init);
	err = Wyslij_Blok_Kamera(OV5642_RGB_QVGA);					//150ms
	if (err)
		return err;
	//Wyslij_I2C_Kamera(0x4300, 0x61);
	//Wyslij_I2C_Kamera(0x4745, 0x02);	//data order [1:0]: 0=D[9:0], 1=D[7:0],D[9:8], 2=D[1:0],D[9:2]
	//Wyslij_I2C_Kamera(0x4741, 0x07);	//test pattern [2] test pattern enable, [1] pattern 0/1, [0] 0=pattern 10-bit, 1=8-bit
	//Wyslij_I2C_Kamera(0x4741 , 0x4);	//test pattern: DLI

	Wyslij_I2C_Kamera(0x5001, 0x7F);	//ISP control 01: [7] Special digital effects, [6] UV adjust enable, [5]1=Vertical scaling enable, [4]1=Horizontal scaling enable, [3] Line stretch enable, [2] UV average enable, [1] color matrix enable, [0] auto white balance AWB
	Wyslij_I2C_Kamera(0x3804, 0x02);	//Timing HW: [3:0] Horizontal width high byte 0x500=1280 (scale input} 0x280=640
	Wyslij_I2C_Kamera(0x3805, 0x80);	//Timing HW: [7:0] Horizontal width low byte
	Wyslij_I2C_Kamera(0x3806, 0x01);	//Timing VH: [3:0] HREF vertical height high byte 0x3C0=960, 0x1E0=480
	Wyslij_I2C_Kamera(0x3807, 0xE0);	//Timing VH: [7:0] HREF vertical height low byte

	//Wyslij_I2C_Kamera(0x3818, 0x80);	//TIMING TC REG18: [6] mirror, [5] Vertial flip, [4] 1=thumbnail mode enabled, [3] 1=compression enabled, [1] vertical subsample 1/4, [0] vertical subsample 1/2  <def:0x80>
	//Wyslij_I2C_Kamera(0x503d , 0x85);	//test pattern: B/W square
	//Wyslij_I2C_Kamera(0x503e, 0x1a);
	//Wyslij_I2C_Kamera(0x5001, 0x4F);	//ISP control 01: [7] Special digital effects, [6] UV adjust enable, [5]1=Vertical scaling enable, [4]1=Horizontal scaling enable, [3] Line stretch enable, [2] UV average enable, [1] color matrix enable, [0] auto white balance AWB

	Wyslij_I2C_Kamera(0x4300, 0x6F);	//format control [7..4] 6=RGB656, [3..0] 1={R[4:0], G[5:3]},{G[2:0}, B[4:0]}


	//konfiguracja DCMI
	hdcmi.Instance = DCMI;
	hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
	hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_FALLING;
	hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_LOW;
	hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_HIGH;
	hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
	hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
	hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
	hdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
	hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
	hdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
	hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;
	hdcmi.Instance->IER = DCMI_IT_FRAME | DCMI_IT_OVR | DCMI_IT_ERR | DCMI_IT_VSYNC | DCMI_IT_LINE;
	err = HAL_DCMI_Init(&hdcmi);
	if (err)
		return err;

    /* DCMI Init */
    hdma_dcmi.Instance = DMA1_Stream0;
    hdma_dcmi.Init.Request = DMA_REQUEST_DCMI;
    hdma_dcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_dcmi.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dcmi.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_dcmi.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_dcmi.Init.Mode = DMA_NORMAL;
    hdma_dcmi.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_dcmi.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    err = HAL_DMA_Init(&hdma_dcmi);
	if (err)
		return err;

	//Konfiguracja transferu DMA z DCMI do pamięci
	return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)nBuforKamery, ROZM_BUF32_KAM);
	//return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)nBuforKamery, ROZM_BUF32_KAM); */
}



HAL_StatusTypeDef InitKamera5(void)
{
	HAL_StatusTypeDef err;

	TIM14->CR1 |= TIM_CR1_CEN;		//włącz timer taktujacy kamerę
	TIM14->ARR = KAMERA_DZIELNIK_ZEGARA - 1;	//częstotliwość PWM
	TIM14->CCR1 = KAMERA_WYPELN_PWM;			//wypełnienie PWM
	TIM14->CCER |= TIM_CCER_CC1E;	//włącz wyjście timera

	Wyslij_I2C_Kamera(0x3103, 0x93);	//PLL clock select: [1] PLL input clock: 1=from pre-divider
	Wyslij_I2C_Kamera(0x3008, 0x82);	//system control 00: [7] software reset mode, [6] software power down mode {def=0x02}
	HAL_Delay(100);
	//Wyslij_Blok_Kamera(ov5642_dvp_fmt_global_init);
	err = Wyslij_Blok_Kamera(OV5642_RGB_QVGA);					//150ms
	if (err)
		return err;
	//Wyslij_I2C_Kamera(0x4300, 0x61);
	//Wyslij_I2C_Kamera(0x4745, 0x02);	//data order [1:0]: 0=D[9:0], 1=D[7:0],D[9:8], 2=D[1:0],D[9:2]
	//Wyslij_I2C_Kamera(0x4741, 0x07);	//test pattern [2] test pattern enable, [1] pattern 0/1, [0] 0=pattern 10-bit, 1=8-bit
	//Wyslij_I2C_Kamera(0x4741 , 0x4);	//test pattern: DLI

	Wyslij_I2C_Kamera(0x5001, 0x7F);	//ISP control 01: [7] Special digital effects, [6] UV adjust enable, [5]1=Vertical scaling enable, [4]1=Horizontal scaling enable, [3] Line stretch enable, [2] UV average enable, [1] color matrix enable, [0] auto white balance AWB
	Wyslij_I2C_Kamera(0x3804, 0x05);	//Timing HW: [3:0] Horizontal width high byte 0x500=1280,  0x280=640, 0x140=320 (scale input}
	Wyslij_I2C_Kamera(0x3805, 0x00);	//Timing HW: [7:0] Horizontal width low byte
	Wyslij_I2C_Kamera(0x3806, 0x03);	//Timing VH: [3:0] HREF vertical height high byte 0x3C0=960, 0x1E0=480, 0x0F0=240
	Wyslij_I2C_Kamera(0x3807, 0xC0);	//Timing VH: [7:0] HREF vertical height low byte

	Wyslij_I2C_Kamera(0x4300, 0x6F);	//format control [7..4] 6=RGB656, [3..0] 1={R[4:0], G[5:3]},{G[2:0}, B[4:0]}

	//konfiguracja DCMI
	hdcmi.Instance = DCMI;
	hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
	hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_FALLING;
	hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_LOW;
	hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_HIGH;
	hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
	hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
	hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
	hdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
	hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
	hdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
	hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;
	hdcmi.Instance->IER = DCMI_IT_FRAME | DCMI_IT_OVR | DCMI_IT_ERR | DCMI_IT_VSYNC | DCMI_IT_LINE;
	err = HAL_DCMI_Init(&hdcmi);
	if (err)
		return err;

    /* DCMI Init */
    hdma_dcmi.Instance = DMA1_Stream0;
    hdma_dcmi.Init.Request = DMA_REQUEST_DCMI;
    hdma_dcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_dcmi.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dcmi.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_dcmi.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_dcmi.Init.Mode = DMA_NORMAL;
    hdma_dcmi.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_dcmi.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    err = HAL_DMA_Init(&hdma_dcmi);
	if (err)
		return err;

	//Konfiguracja transferu DMA z DCMI do pamięci
	return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)nBuforKamery, ROZM_BUF32_KAM);
	//return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)nBuforKamery, ROZM_BUF32_KAM); */
}



////////////////////////////////////////////////////////////////////////////////
// wykonuje jedno zdjęcie kamerą i trzyma je w buforze kamery
// Parametry:
// [i] - sSzerokosc - szerokość zdjecia w pikselach
// [i] - sWysokosc - wysokość zdjęcia w pikselach
// Zwraca: kod błędu
////////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef ZrobZdjecie(int16_t sSzerokosc, uint16_t sWysokosc)
{
	HAL_StatusTypeDef err;

	err = HAL_DCMI_Stop(&hdcmi);
	if (err)
		return err;

	//skalowanie obrazu
	Wyslij_I2C_Kamera(0x5001, 0x7F);	//ISP control 01: [7] Special digital effects, [6] UV adjust enable, [5]1=Vertical scaling enable, [4]1=Horizontal scaling enable, [3] Line stretch enable, [2] UV average enable, [1] color matrix enable, [0] auto white balance AWB
	Wyslij_I2C_Kamera(0x3804, 0x05);	//Timing HW: [3:0] Horizontal width high byte 0x500=1280,  0x280=640, 0x140=320 (scale input}
	Wyslij_I2C_Kamera(0x3805, 0x00);	//Timing HW: [7:0] Horizontal width low byte
	Wyslij_I2C_Kamera(0x3806, 0x03);	//Timing VH: [3:0] HREF vertical height high byte 0x3C0=960, 0x1E0=480, 0x0F0=240
	Wyslij_I2C_Kamera(0x3807, 0xC0);	//Timing VH: [7:0] HREF vertical height low byte

	//ustaw rozmiar obrazu
	Wyslij_I2C_Kamera(0x3808, (sSzerokosc & 0xFF00)>>8);	//Timing DVPHO: [3:0] output horizontal width high byte [11:8]
	Wyslij_I2C_Kamera(0x3809, (sSzerokosc & 0x00FF));		//Timing DVPHO: [7:0] output horizontal width low byte [7:0]
	Wyslij_I2C_Kamera(0x380a, (sWysokosc & 0xFF00)>>8);		//Timing DVPVO: [3:0] output vertical height high byte [11:8]
	Wyslij_I2C_Kamera(0x380b, (sWysokosc & 0x00FF));		//Timing DVPVO: [7:0] output vertical height low byte [7:0]

	Wyslij_I2C_Kamera(0x4300, 0x6F);	//format control [7..4] 6=RGB656, [3..0] 1={R[4:0], G[5:3]},{G[2:0}, B[4:0]}

	//Konfiguracja transferu DMA z DCMI do pamięci
	return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)nBuforKamery, ROZM_BUF32_KAM);
}

HAL_StatusTypeDef ZrobZdjecie2(int16_t sSzerokosc, uint16_t sWysokosc, uint8_t rej)
{
	HAL_StatusTypeDef err;

	err = HAL_DCMI_Stop(&hdcmi);
	if (err)
		return err;

	//skalowanie obrazu
	Wyslij_I2C_Kamera(0x5001, 0x7F);	//ISP control 01: [7] Special digital effects, [6] UV adjust enable, [5]1=Vertical scaling enable, [4]1=Horizontal scaling enable, [3] Line stretch enable, [2] UV average enable, [1] color matrix enable, [0] auto white balance AWB
	Wyslij_I2C_Kamera(0x3804, 0x05);	//Timing HW: [3:0] Horizontal width high byte 0x500=1280,  0x280=640, 0x140=320 (scale input}
	Wyslij_I2C_Kamera(0x3805, 0x00);	//Timing HW: [7:0] Horizontal width low byte
	Wyslij_I2C_Kamera(0x3806, 0x03);	//Timing VH: [3:0] HREF vertical height high byte 0x3C0=960, 0x1E0=480, 0x0F0=240
	Wyslij_I2C_Kamera(0x3807, 0xC0);	//Timing VH: [7:0] HREF vertical height low byte

	//ustaw rozmiar obrazu
	Wyslij_I2C_Kamera(0x3808, (sSzerokosc & 0xFF00)>>8);	//Timing DVPHO: [3:0] output horizontal width high byte [11:8]
	Wyslij_I2C_Kamera(0x3809, (sSzerokosc & 0x00FF));		//Timing DVPHO: [7:0] output horizontal width low byte [7:0]
	Wyslij_I2C_Kamera(0x380a, (sWysokosc & 0xFF00)>>8);		//Timing DVPVO: [3:0] output vertical height high byte [11:8]
	Wyslij_I2C_Kamera(0x380b, (sWysokosc & 0x00FF));		//Timing DVPVO: [7:0] output vertical height low byte [7:0]

	Wyslij_I2C_Kamera(0x4300, rej);	//format control [7..4] 6=RGB656, [3..0] 1={R[4:0], G[5:3]},{G[2:0}, B[4:0]}

	//Konfiguracja transferu DMA z DCMI do pamięci
	return HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)nBuforKamery, ROZM_BUF32_KAM);
}

////////////////////////////////////////////////////////////////////////////////
// Odczytuje zadany rejestr kamery i zwraca do wyższych warstw oprogramowania
// Parametry: brak
// Zwraca: zawartość rejestru
////////////////////////////////////////////////////////////////////////////////
uint8_t CzytajKamInit(void)
{
	uint8_t dane;
	//Czytaj_I2C_Kamera(0x4740, &dane);	//Polarity CTRL00
	Czytaj_I2C_Kamera(0x4300, &dane);	//format control
	return dane;
}



////////////////////////////////////////////////////////////////////////////////
// Wyślij polecenie konfiguracyjne do kamery przez I2C4. Funkcja wysyłajaca jest blokująca, kończy się po wysłaniu danych.
// Parametry:
//  rejestr - 16 bitowy adres rejestru kamery
//  dane - dane zapisywane do rejestru
// Zwraca: kod błędu HAL
////////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef Wyslij_I2C_Kamera(uint16_t rejestr, uint8_t dane)
{
	uint8_t dane_wy[3];

	dane_wy[0] = (rejestr & 0xFF00) >> 8;
	dane_wy[1] = (rejestr & 0x00FF);
	dane_wy[2] = dane;
	return HAL_I2C_Master_Transmit(&hi2c4, OV5642_I2C_ADR, dane_wy, 3, KAMERA_TIMEOUT);
}



////////////////////////////////////////////////////////////////////////////////
// Odczytaj dane z rejestru kamery
// Parametry:
//  rejestr - 16 bitowy adres rejestru kamery
//  dane - dane zapisywane do rejestru
// Zwraca: kod błędu HAL
////////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef Czytaj_I2C_Kamera(uint16_t rejestr, uint8_t *dane)
{
	uint8_t dane_wy[2];
	HAL_StatusTypeDef err;

	dane_wy[0] = (rejestr & 0xFF00) >> 8;
	dane_wy[1] = (rejestr & 0x00FF);
	err = HAL_I2C_Master_Transmit(&hi2c4, OV5642_I2C_ADR, dane_wy, 2, KAMERA_TIMEOUT);
	if (err == 0)
		err = HAL_I2C_Master_Receive(&hi2c4, OV5642_I2C_ADR, dane, 1, KAMERA_TIMEOUT);
	return err;
}


////////////////////////////////////////////////////////////////////////////////
// Wyślij polecenie konfiguracyjne do kamery przez I2C4. Funkcja wysyłajaca jest blokująca, kończy się po wysłaniu danych.
// Parametry:
//  rejestr - 16 bitowy adres rejestru kamery
//  dane - dane zapisywane do rejestru
// Zwraca: kod błędu HAL
////////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef Wyslij_Blok_Kamera(const struct sensor_reg reglist[])
{
	const struct sensor_reg *next = reglist;
	HAL_StatusTypeDef err;

	while ((next->reg != 0xFFFF) && (err == 0))
	{
		err = Wyslij_I2C_Kamera(next->reg, next->val);
		next++;
	}
	return err;
}



////////////////////////////////////////////////////////////////////////////////
// Callback od końca wiersza kamery
// Parametry:
//  hdcmi - wskaźnik na interfejs DCMI
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	sLicznikLiniiKamery++;
}



////////////////////////////////////////////////////////////////////////////////
// Callback od końca wiersza kamery
// Parametry:
//  hdcmi - wskaźnik na interfejs DCMI
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{

}



////////////////////////////////////////////////////////////////////////////////
// Callback od końca wiersza kamery
// Parametry:
//  hdcmi - wskaźnik na interfejs DCMI
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	chNowyObrazKamery = 1;
	sLicznikLiniiKamery= 0;
}


void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi)
{
}


////////////////////////////////////////////////////////////////////////////////
// konwersja bufora w formacie RAW na RGB656
// Parametry:
//  nBuforKamery - wskaźnik na bufor danych do konwersji
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void RAW2RGB(uint32_t *nBufKamery, uint16_t *sBufLCD)
{
	uint8_t pixel1[3], pixel2[3];
	uint16_t x, y;
	uint32_t dx, dy0, dy1;
	uint32_t pk1, pk2;

	for (y=0; y<LCD_WYS; y++)
		for (x=0; x<LCD_SZER; x++)
			sBufLCD[y * LCD_SZER + x] = 0;

	for (y=0; y<LCD_WYS; y++)
	{
		dy0 = (2*y + 0) * LCD_SZER/2;
		dy1 = (2*y + 1) * LCD_SZER/2;
		for (x=0; x<LCD_SZER/2; x++)
		{
			dx = 2*x;
			pk1 = nBufKamery[dy0 + x];
			pk2 = nBufKamery[dy1 + x];

			/*pixel1[COL_B] = (pk1 & 0x000000FF);
			pixel2[COL_B] = (pk1 & 0x00FF0000) >> 16;

			pixel1[COL_R] = (pk2 & 0x00FF0000) >> 16;
			pixel2[COL_R] = (pk2 & 0x000000FF);

			pixel1[COL_G] = (((pk1 & 0xFF000000) >> 24) + ((pk2 & 0x00FF0000) >> 16)) / 2;
			pixel2[COL_G] = (((pk1 & 0x0000FF00) >> 8)  + (pk2 & 0x000000FF)) / 2; */

			pixel1[COL_B] = (pk1 & 0x0000FF00) >> 8;
			pixel2[COL_B] = (pk1 & 0xFF000000) >> 24;

			pixel1[COL_R] = (pk2 & 0x000000FF);
			pixel2[COL_R] = (pk2 & 0x00FF0000) >> 16;

			pixel1[COL_G] = (((pk2 & 0x0000FF00) >> 8)  + (pk1 & 0x000000FF)) / 2;
			pixel2[COL_G] = (((pk2 & 0xFF000000) >> 24) + ((pk1 & 0x00FF0000) >> 16)) / 2;

			//formatowanie danych wyjściowych
			sBufLCD[y * LCD_SZER + dx + 0] = ((pixel1[COL_R] & 0xF8) << 8) | ((pixel1[COL_G] & 0xFC) << 3) | ((pixel1[COL_B] & 0xF8) >> 3);
			sBufLCD[y * LCD_SZER + dx + 1] = ((pixel2[COL_R] & 0xF8) << 8) | ((pixel2[COL_G] & 0xFC) << 3) | ((pixel2[COL_B] & 0xF8) >> 3);
		}
	}
}

// set the output size
////////////////////////////////////////////////////////////////////////////////
// konwersja bufora w formacie RAW na RGB656
// Parametry:
//  nBuforKamery - wskaźnik na bufor danych do konwersji
// Zwraca: nic
////////////////////////////////////////////////////////////////////////////////
void OV5642_OutSize_Set(uint16_t offX, uint16_t offY, uint16_t width, uint16_t height)
{
	Wyslij_I2C_Kamera(0x3212,0x03);

	Wyslij_I2C_Kamera(0x3808, width >> 8);
	Wyslij_I2C_Kamera(0x3809, width & 0xFF);
	Wyslij_I2C_Kamera(0x380a, height >> 8);
	Wyslij_I2C_Kamera(0x380b, height & 0xFF);

	Wyslij_I2C_Kamera(0x3810, offX >> 8);
	Wyslij_I2C_Kamera(0x3811, offX & 0xFF);

	Wyslij_I2C_Kamera(0x3812, offY >> 8);
	Wyslij_I2C_Kamera(0x3813, offY & 0xFF);

	Wyslij_I2C_Kamera(0x3212, 0x13);
	Wyslij_I2C_Kamera(0x3212, 0xa3);
}




////////////////////////////////////////////////////////////////////////////////
// Sprawdź czy mamy kontakt z kamerą
// Parametry: brak
// Zwraca: systemowy kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t	SprawdzKamere(void)
{
	uint16_t DaneH;
	uint8_t daneL, powtorz = 10;

	do
	{
		Czytaj_I2C_Kamera(0x300A, (uint8_t*)&DaneH);	//Chip ID High Byte = 0x56
		Czytaj_I2C_Kamera(0x300B, &daneL);	//Chip ID Low Byte = 0x42
		powtorz--;
		HAL_Delay(1);
		DaneH <<= 8;
		DaneH |= daneL;
	}
	while ((DaneH != OV5642_ID) && powtorz);
	if (powtorz == 0)
		return ERR_BRAK_KAMERY;
	else
		return ERR_OK;
}

