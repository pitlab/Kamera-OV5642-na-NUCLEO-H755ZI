//////////////////////////////////////////////////////////////////////////////
//
// AutoPitLot v2.0
// Obs�uga magnetometru HMC5883 na magistrali I2C
//
// (c) Pit Lab
// http://www.pitlab.pl
//////////////////////////////////////////////////////////////////////////////
#include "HMC5883.h"
#include "..\..\..\Common\Inc\errcode.h"


// Obs�uga magnetometru wymaga wykonania kilku czynno�ci roz�o�onych w czasie
// 1) Wystartowanie konwersji trwaj�cej 6ms (slot czasowy 0)
// 2) Wystartowanie odczytu wykonanego pomiaru. Odczyt wykonuje si� w tle w procedurze przerwania I2C i 
//    trwa ok. 4ms (slot 11)
// 3) Przepisania odczytanych w przerwaniu danych do zmiennych pomiarowych

//Czas od rozpocz�cia pomiaru do gotowych danych 6ms

////////////////////////////////////////////////////////////////////////////////
// Budzi układ i startuje konwersję magnetometru HMC5883
// Dane powinny pojawią się 6ms po starcie konwersji
// Parametry: brak
// Zwraca: kod błędu HAL
// Czas zajęcia magistrali I2C: 760us przy zegarze 100kHz
////////////////////////////////////////////////////////////////////////////////
uint8_t StartMagnetometerHMC(void)
{
    uint8_t dane_we[2];

    dane_we[0] = MODE;
    dane_we[1] = (1 << 0);   //Mode Select:0=Continuous-Measurement Mode, 1=Single-Measurement Mode, 2-3=Idle Mode.

    return HAL_I2C_Master_Transmit(&hi2c1, HMC_I2C_ADR, dane_we, 2, MAG_TIMEOUT);		//rozpocznij pomiar na I2C1
}



////////////////////////////////////////////////////////////////////////////////
// Inicjuje odczyt danych z magnetometru HMC5883
// Parametry: *dane_wy wskaźnik na dane wychodzące
// Zwraca: kod błędu HAL
// Czas zajęcia magistrali I2C: 2,2ms przy zegarze 100kHz
////////////////////////////////////////////////////////////////////////////////
uint8_t ReadMagnetometerHMC(float *fWynik)
{
	uint8_t dane[6];
	uint8_t err;

    dane[0] = DATA_XH;
    err = HAL_I2C_Master_Transmit(&hi2c1, HMC_I2C_ADR, dane, 1, MAG_TIMEOUT);	//wyślij polecenie odczytu wszystkich pomiarów
    if (err)
    	return err;

    err =  HAL_I2C_Master_Receive(&hi2c1, HMC_I2C_ADR, dane, 6, MAG_TIMEOUT);		//odczytaj dane
    if (err)
       	return err;

    if ((!dane[0] && !dane[1]) | (!dane[2] && !dane[3]) | (!dane[4] && !dane[5]))
    	return ERR_ZLE_DANE;

    *(fWynik+0) = 0x100 * dane[0] + dane[1];
	*(fWynik+1) = 0x100 * dane[2] + dane[3];
	*(fWynik+2) = 0x100 * dane[4] + dane[5];
	return ERR_OK;
}







////////////////////////////////////////////////////////////////////////////////
// Inicjuje zmienne konfiguracyjne
// Parametry: 
// Zwraca: kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t InitMagnetometerHMC(void)
{
    uint8_t dane[4];

    //sprawdź obecność magnetometru na magistrali I2C2
    if (TestPresenceHMC5883() == ERR_BRAK_MAGN)
    	return ERR_BRAK_MAGN;

    dane[0] = CONF_A;
    //Ustaw tryb pracy na 30Hz
    //Configuration Register A
    dane[1] = (0 << 0)|   //Measurement Configuration: 0=Normal measurement configuration, 1=Positive bias configuration, 2=Negative bias configuration
              (5 << 2)|   //Data Output Rate:6=75Hz, 5=30, 4=15, 3=7,5, 2=3
              (2 << 5)|   //Select number of samples averaged per measurement output: 00 = 1; 01 = 2; 10 = 4; 11 = 8
              (0 << 7);   //This bit must be cleared for correct operation.
    //Configuration Register B
    dane[2] = (1 << 5);   //Gain Configuration: 0=0,88 Gaussa, 1=1,3; 2=1,9; , 3=2,5; 4=4; 5=4,7; 6=5,6; 7=8,1 Gaussa
    //Mode Register
    dane[3] = (0 << 0)|   //Mode Select:0=Continuous-Measurement Mode, 1=Single-Measurement Mode, 2-3=Idle Mode.
              (0 << 2);   //Bits 2-7 must be cleared for correct operation.

    return HAL_I2C_Master_Transmit(&hi2c1, HMC_I2C_ADR, dane, 4, MAG_TIMEOUT);		//zapisz 3 rejestry jedną transmisją
}


////////////////////////////////////////////////////////////////////////////////
// Testuje obecność magnetometru HMC5883 na magistrali korzystając z Identification Register A,B,C
// Parametry: brak
// Zwraca: systemowy kod błędu
// Czas zajęcia magistrali I2C: ms przy zegarze 100kHz
////////////////////////////////////////////////////////////////////////////////
uint8_t TestPresenceHMC5883(void)
{
	uint8_t err;
    uint8_t	dane[3];

    dane[0] = ID_A;
    err = HAL_I2C_Master_Transmit(&hi2c1, HMC_I2C_ADR, dane, 1, MAG_TIMEOUT);	//wyślij polecenie odczytu rejestrów identyfikacyjnych
    if (!err)
    {
        err =  HAL_I2C_Master_Receive(&hi2c1, HMC_I2C_ADR, dane, 3, MAG_TIMEOUT);		//odczytaj dane
        if (!err)
        {
        	  if ((dane[0] == 'H') && (dane[1] == '4') && (dane[2] == '3'))
        		  return ERR_OK;
        }
    }
    return ERR_BRAK_MAGN;
}
