#include "cobs_protokol.h"
#include "serijska.h"
/*------------------------------PORUKE----------------------------------------------
Odgovor OK-----> dekodirano-> 0x01--------> kodirano-> 0x02 0x01 0x00
Odgovor GRESKA-> dekodirano-> 0x00--------> kodirano-> 0x01 0x01 0x00
START----------> dekodirano-> 0x01 0x01---> kodirano-> 0x03 0x01 0x01 0x00
STOP-----------> dekodirano-> 0x01 0x00---> kodirano-> 0x02 0x01 0x01 0x00
N:0------------> dekodirano-> 0x02 0x00---> kodirano-> 0x02 0x02 0x01 0x00
N:1------------> dekodirano-> 0x02 0x01---> kodirano-> 0x03 0x02 0x01 0x00
N:2------------> dekodirano-> 0x02 0x02---> kodirano-> 0x03 0x02 0x02 0x00
N:3------------> dekodirano-> 0x02 0x03---> kodirano-> 0x03 0x02 0x03 0x00
N:4------------> dekodirano-> 0x02 0x04---> kodirano-> 0x03 0x02 0x04 0x00
N:5------------> dekodirano-> 0x02 0x05---> kodirano-> 0x03 0x02 0x05 0x00
N:6------------> dekodirano-> 0x02 0x06---> kodirano-> 0x03 0x02 0x06 0x00
N:7------------> dekodirano-> 0x02 0x07---> kodirano-> 0x03 0x02 0x07 0x00
N:8------------> dekodirano-> 0x02 0x08---> kodirano-> 0x03 0x02 0x08 0x00
N:9------------> dekodirano-> 0x02 0x09---> kodirano-> 0x03 0x02 0x09 0x00
--------------------------------------------------------------*/
uint8_t start_flag = 0;
uint8_t nitna_flag = 0;

uint8_t odgovor_OK[] = {0x01};
uint8_t odgovor_GRESKA[] = {0x00};

uint8_t* kodiraj_poruku(uint8_t* poruka, uint8_t duzina_poruke) {
	uint8_t i, indeks_za_upis_nule = 0;
	uint8_t kodirana_poruka[MAKS_DUZINA_PORUKE];

	for(i = 0; i < duzina_poruke; i++){
		// Ukoliko je trenutni bajt 0, na mesto
		// prethodne 0 se upisuje broj bajtova izmedju
		// prethodne i trenutne 0 
		if(poruka[i] == 0){
			kodirana_poruka[indeks_za_upis_nule] = (i + 1) - indeks_za_upis_nule;
			indeks_za_upis_nule = i + 1;
		}
		else {
			kodirana_poruka[i + 1] = poruka[i];
		}
	}
	kodirana_poruka[indeks_za_upis_nule] = (duzina_poruke + 1) - indeks_za_upis_nule;
	kodirana_poruka[duzina_poruke+1] = 0;

	return kodirana_poruka;
}

uint8_t* dekodiraj_poruku(uint8_t* kodirana_poruka) {
	uint8_t dekodirana_poruka[MAKS_DUZINA_PORUKE];
	uint8_t duzina_poruke = 0, i = 0, j = 0, pozicija_nule;

	while (kodirana_poruka[i] != KRAJ_PORUKE) {
		duzina_poruke++;
		i++;
	}
	duzina_poruke++;

	pozicija_nule = kodirana_poruka[0];
	for (i = 1; i < duzina_poruke; i++) {
		if (i == pozicija_nule) {
			dekodirana_poruka[j++] = 0x00;
			pozicija_nule = i + kodirana_poruka[i];
		}
		else {
			dekodirana_poruka[j++] = kodirana_poruka[i];
		}
	}

	return dekodirana_poruka;
}

void parsiraj_poruku(uint8_t* poruka) {
	uint8_t* dekodirana_poruka = dekodiraj_poruku(poruka);

	if(dekodirana_poruka[0]==0x01 && dekodirana_poruka[1]==0x00){
		start_flag = 0;
		posalji_poruku(odgovor_OK, 1);
		//STOP
	}else if(dekodirana_poruka[0]==0x01 && dekodirana_poruka[1]==0x01){
		start_flag = 1;
		posalji_poruku(odgovor_OK, 1);
		//START
	}else if(dekodirana_poruka[0]==0x02 && dekodirana_poruka[1]==0x00){
		nitna_flag = 0;
		posalji_poruku(odgovor_OK, 1);		
	}else if(dekodirana_poruka[0]==0x02 && dekodirana_poruka[1]==0x01){
		nitna_flag = 1;
		posalji_poruku(odgovor_OK, 1);		
	}else if(dekodirana_poruka[0]==0x02 && dekodirana_poruka[1]==0x02){
		nitna_flag = 2;
		posalji_poruku(odgovor_OK, 1);		
	}else if(dekodirana_poruka[0]==0x02 && dekodirana_poruka[1]==0x03){
		nitna_flag = 3;
		posalji_poruku(odgovor_OK, 1);		
	}else if(dekodirana_poruka[0]==0x02 && dekodirana_poruka[1]==0x04){
		nitna_flag = 4;
		posalji_poruku(odgovor_OK, 1);		
	}else if(dekodirana_poruka[0]==0x02 && dekodirana_poruka[1]==0x05){
		nitna_flag = 5;
		posalji_poruku(odgovor_OK, 1);		
	}else if(dekodirana_poruka[0]==0x02 && dekodirana_poruka[1]==0x06){
		nitna_flag = 6;
		posalji_poruku(odgovor_OK, 1);		
	}else if(dekodirana_poruka[0]==0x02 && dekodirana_poruka[1]==0x07){
		nitna_flag = 7;
		posalji_poruku(odgovor_OK, 1);		
	}else if(dekodirana_poruka[0]==0x02 && dekodirana_poruka[1]==0x08){
		nitna_flag = 8;
		posalji_poruku(odgovor_OK, 1);		
	}else if(dekodirana_poruka[0]==0x02 && dekodirana_poruka[1]==0x09){
		nitna_flag = 9;
		posalji_poruku(odgovor_OK, 1);		
	}else {
		posalji_poruku(odgovor_GRESKA, 1);
	}
}