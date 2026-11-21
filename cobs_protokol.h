#ifndef PROTOKOL_H
#define PROTOKOL_H

#include "tipovi.h"

#define MAKS_DUZINA_PORUKE 20
extern uint8_t start_flag;
extern uint8_t nitna_flag;

uint8_t* kodiraj_poruku(uint8_t* poruka, uint8_t duzina_poruke);
uint8_t* dekodiraj_poruku(uint8_t* kodirana_poruka);
void parsiraj_poruku(uint8_t* poruka);

#endif