//////////////////////////////////////////////////////////////////////////////
//
// AutoPitLot v3.0
// Moduł obsługi modułu ethernet
//
// (c) PitLab 2024
// http://www.pitlab.pl
//////////////////////////////////////////////////////////////////////////////

#include "ethernet.h"

#include "..\..\..\Common\Inc\errcode.h"

////////////////////////////////////////////////////////////////////////////////
// Inicjalizuje moduł ethernetowy
// Parametry: *pEth - wskaźnik na instancję modułu ethetrnetowego
// Zwraca: systemowy kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t  InitEth(ETH_HandleTypeDef* pEth)
{
	//HAL_ETH_StateTypeDef stan;
	//ETH_InitTypeDef sEthInit;
	//sEthInit = pEth->Init();




	//stan = pEth->gState;
	HAL_ETH_Start(pEth);
	//stan = pEth->gState;
	//ETH_DMADESCTTypeDef DMARxDescTabp
	//pEth->
	return ERR_OK;
}


////////////////////////////////////////////////////////////////////////////////
// Odczytuje ramk ethernetową
// Parametry: *pEth - wskaźnik na instancję modułu ethetrnetowego
// Zwraca: systemowy kod błędu
////////////////////////////////////////////////////////////////////////////////
uint8_t  AnalizujEth(ETH_HandleTypeDef* pEth)
{
	static uint8_t chPakiet[ETH_MAX_PACKET_SIZE];
	//uint32_t nRozmiar;
	HAL_ETH_ReadData(pEth, (void **)&chPakiet);

	WyswietlAnalizeEth(&chPakiet[8], &chPakiet[14]);
	return ERR_OK;
}
