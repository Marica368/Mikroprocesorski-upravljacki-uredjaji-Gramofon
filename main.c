/*
GRAMOFON 1
Implemetirati program za upravljanje maketom gramofona. Gramofon ima jedan ulaz i dva izlaza.
Ulaz:  
	Start sistema  
Izlazi:			   
	[P0_2] Prolazak nitne ispod induktivnog davaca
	[P0_3] Detektovan krug motora

Uredjaj moze da radi u dva moda:
	1) Brojanje nitni - Uredjaj se startuje i svaki put kad detektuje krug motora na displeju ispise broj
nitni na gramofonu kao i vreme za koje napravi pun krug.
	2) Pomeranje gramofona za odreden broj nitni - Uredjaju se posalje komanda da se pomeri za
odredjeni broj nitni. Na displeju ispisati vreme (stavio zastitu da to sme da bude max 60s) za koje se 
gramofon pomerao.
Komande koje mogu da se posalju serijskom komunikacijom su:
~ Start
~ Stop
~ Pomeraj za odredjen broj nitni
Na displeju ispisati u prvom redu status gramofona a u drugom odgovarajuce parametre. 
*/

#include "tipovi.h"
#include "gpio.h"
#include "prekidi.h"
#include "serijska.h"
#include "cobs_protokol.h"
#include "display.h"

uint8_t mod;

int8_t status;
int16_t brojac_prekida;
uint8_t tajmer = 0;

uint8_t brojacZaTaster=0;
uint8_t trenutno_hardversko_stanje_P0_0=1;
uint8_t prethodno_hardversko_stanje_P0_0=1;
uint8_t trenutno_softversko_P0_0=1;

uint8_t trenutno_hardversko_stanje_P0_1=1;
uint8_t prethodno_hardversko_stanje_P0_1=1;
uint8_t trenutno_softversko_P0_1=1;

uint8_t trenutno_hardversko_stanje_P0_2=1;
uint8_t prethodno_hardversko_stanje_P0_2=1;
uint8_t trenutno_softversko_P0_2=1;

uint8_t trenutno_hardversko_stanje_P0_3=1;
uint8_t prethodno_hardversko_stanje_P0_3=1;
uint8_t trenutno_softversko_P0_3=1;

void interrupt_t1(void) interrupt 3 {  
	TL1 = 0x48;
	TH1 = 0x48;
	if (++brojac_prekida == 4650) {
		tajmer += 1;	  			// svaki put kad tajmer izbroji 1s
		status = 1;
		brojac_prekida = 0;
	}
	//DEBOUNCING
   //P0_0 taster  ' start / stop '
   trenutno_hardversko_stanje_P0_0=P0_0;
   if(trenutno_hardversko_stanje_P0_0 == prethodno_hardversko_stanje_P0_0){
		if(++brojacZaTaster == 5){
			trenutno_softversko_P0_0 = trenutno_hardversko_stanje_P0_0;
			brojacZaTaster=0;
		}
	}
	else{
	 	brojacZaTaster=0;
	}
	prethodno_hardversko_stanje_P0_0 = trenutno_hardversko_stanje_P0_0;

	//P0_1 taster	' mod1 / mod2 '
	trenutno_hardversko_stanje_P0_1=P0_1;
	if(trenutno_hardversko_stanje_P0_1 == prethodno_hardversko_stanje_P0_1){
		if(++brojacZaTaster == 5){
			trenutno_softversko_P0_1 = trenutno_hardversko_stanje_P0_1;
			brojacZaTaster=0;
		}
	}
	else{
	 	brojacZaTaster=0;
	}
	prethodno_hardversko_stanje_P0_1 = trenutno_hardversko_stanje_P0_1;

	//P0_2 taster	' detektovana nitna '
	trenutno_hardversko_stanje_P0_2=P0_2;
	if(trenutno_hardversko_stanje_P0_2 == prethodno_hardversko_stanje_P0_2){
		if(++brojacZaTaster == 5){
			trenutno_softversko_P0_2 = trenutno_hardversko_stanje_P0_2;
			brojacZaTaster=0;
		}
	}
	else{
	 	brojacZaTaster=0;
	}
	prethodno_hardversko_stanje_P0_2 = trenutno_hardversko_stanje_P0_2;

	//P0_3 taster	' pun krug motora '
	trenutno_hardversko_stanje_P0_3=P0_3;
	if(trenutno_hardversko_stanje_P0_3 == prethodno_hardversko_stanje_P0_3){
		if(++brojacZaTaster == 5){
			trenutno_softversko_P0_3 = trenutno_hardversko_stanje_P0_3;
			brojacZaTaster=0;
		}
	}
	else{
	 	brojacZaTaster=0;
	}
	prethodno_hardversko_stanje_P0_3 = trenutno_hardversko_stanje_P0_3;
}

int8_t *num2str(int16_t broj){
	uint16_t i=0;
	uint16_t j;
	uint16_t ostatak;
	uint16_t len=0;
	uint16_t lenstr=0;
	int8_t str[8];
	int8_t pom[8];
	while(broj!=0){
		ostatak=broj%10; 	//cuva posljednju cifru broja
		broj=broj/10; 	
		pom[i]=ostatak+48;
		len++;
		i++;
	}
	pom[len]='\0';
	lenstr=len;
	j=len-1;
	for(i=0; i<lenstr; i++, j--){
		str[i]=pom[j];
	}
	str[lenstr]='\0';
	return str;
}

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
void main (void)
{
	 uint8_t niz[]={0x01,0x01,0x00};
	 uint8_t nitna = 0;   // brojanje nitni u MOD1
	 uint8_t nitna2 = 0;  // brojanje nitni u MOD2
	 uint8_t trenutno_stanje_P0_0=1;
	 uint8_t prethodno_stanje_P0_0=1;
	 uint8_t trenutno_stanje_P0_1=1;
	 uint8_t prethodno_stanje_P0_1=1;
	 uint8_t trenutno_stanje_P0_2=1;
	 uint8_t prethodno_stanje_P0_2=1;
	 uint8_t trenutno_stanje_P0_3=1;
	 uint8_t prethodno_stanje_P0_3=1;
   
	uint8_t* dekodirana_poruka  =	dekodiraj_poruku(niz);
	
	init_gpio();
	 
    initDisplay();
	
	mod = 0;
	P2=0;

	TL1 = 0x48;
	TH1 = 0x48;
	TMOD = 0x20;
	TR1 = 1;
	ET1 = 1;


	CKCON0 &= 0xFE;
	init_serijska(0x80, 0x50, 0x1C, 253);
	omoguci_sve_prekide();
	omoguci_prekid_serijske(1);

	status = 0;
	brojac_prekida = 0;	  
	 
	while(1){
		// P0_0  START / STOP
		trenutno_stanje_P0_0=trenutno_softversko_P0_0;
		if (trenutno_stanje_P0_0 > prethodno_stanje_P0_0) {
			start_flag=~start_flag;
			if(start_flag){
				brojac_prekida = 0;
				TL1 = 0x48;
				TH1 = 0x48;
				mod = 0;
				nitna = 0;
				nitna2 = 0;
				tajmer = 0;
				
				
				clearDisplay();
				writeLine("MOD 1");
			}
			else{
		
				clearDisplay();
				writeLine("STOP");
				
				mod = 0;
				P2 = 0;
			}
		} 
		 
		// P0_1 MOD1 / MOD2	(mod == 0x00 / 0xFF)
		trenutno_stanje_P0_1=trenutno_softversko_P0_1;
		if (trenutno_stanje_P0_1 > prethodno_stanje_P0_1) {
			P2=0;
			mod=~mod;
			nitna = 0;
			nitna2 = 0;
			tajmer = 0;
			if(mod){
				clearDisplay();
				writeLine("MOD 2");
			}
			else{
			    clearDisplay();
				writeLine("MOD 1");
			}
		}

		// P0_2 Detektovana nitna
		trenutno_stanje_P0_2=trenutno_softversko_P0_2;

		// P0_3 Pun krug
		trenutno_stanje_P0_3=trenutno_softversko_P0_3;
		
		if(start_flag){
			switch(mod){
				// MOD 1  Brojanje nitni
				case 0:
					if(status){
						P2=0xFF;
						if(trenutno_stanje_P0_2 > prethodno_stanje_P0_2){
							nitna+=1;
						}
						if(trenutno_stanje_P0_3 > prethodno_stanje_P0_3){
							//ispis posto je presao pun krug
							clearDisplay();
							writeLine("MOD 1");	
							newLine();
							//ako je proslo vise od 60s
							if(tajmer>60){
								writeLine("timeout");
							}
							else{
								writeLine("n:");
								writeLine(num2str(nitna));
								writeLine(" ");
								writeLine(num2str(tajmer));
								writeLine("s");
							}
							tajmer=0;
							nitna=0;
						}
					}
					break;
				// MOD 2	 Pozicioniranje za odredjen broj nitni
				case 0xFF:
					if(status){
						if(nitna_flag!=0){
							if (trenutno_stanje_P0_2 > prethodno_stanje_P0_2){
								nitna2+=1;
							}
							if(nitna2==nitna_flag){
								P2=0;	  		
								clearDisplay();
								writeLine("MOD 2");	
								newLine();
								if(tajmer>60){
									writeLine("timeout");
								}
								else{
									writeLine(num2str(tajmer));
									writeLine("s");
								}
								tajmer=0;
								nitna2=0;
								nitna_flag=0;
							}
							else{
								P2=0xFF;
							}
						}
						else{
							P2=0x00;
						}
					}
					break;
				default:
					P2=0x00;
					break;
			}
		}
	 
	prethodno_stanje_P0_0 = trenutno_stanje_P0_0;
	prethodno_stanje_P0_1 = trenutno_stanje_P0_1;
	prethodno_stanje_P0_2 = trenutno_stanje_P0_2;
	prethodno_stanje_P0_3 = trenutno_stanje_P0_3;
	
	} 
}