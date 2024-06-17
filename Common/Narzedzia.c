/*
 * Narzedzia.c
 *
 *  Created on: May 24, 2024
 *      Author: PitLab
 */

#include "..\..\..\Common\Inc\errcode.h"

////////////////////////////////////////////////////////////////////////////////
// Makro czekajce na spełnienie warunku lub wykonanie podanej liczby iteracji
// Parametry:
// [i] warunek - testowany warunek
// [i] iteracji - liczba iteracji do wykonania zanim wyjdzie z błędem
// Zwraca: systemowy kod błędu
////////////////////////////////////////////////////////////////////////////////
#define BadajCzyPrawda(warunek, iteracji)	\
{											\
	uint16_t n;								\
	for (n=(granica)); !(warunek); n--)		\
		if (n <= 0)							\
			return ERR_TIMEOUT;				\
	return ERR_OK;							\
}											\



