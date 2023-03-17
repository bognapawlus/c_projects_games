/**                                                                  
 * "SameGame" - zadanie zaliczeniowe 2
 *
 * Program wykonujący jedno polecenie usunięcia klocków w Samegame
 *
 * Korzysta z biblioteki ncurses i programu wykonywalnego `zadanie2`,
 * usuwającego grupę klocków.
 *
 * Wczytuje planszę przed ruchem i wypisuje planszę po ruchu.
 *
 * Kompilacja poleceniem:
 *
 * gcc @opcje -DWIERSZE=10 -DKOLUMNY=15 -DRODZAJE=4 zadanie2.c -o zadanie2
 *
 * Program:
 * - czyta stan planszy;
 * - sprawdza, czy na polu o współrzędnych określonych argumentami programu, 
 *   jest klocek należący do grupy mającej co najmniej dwa elementy i jeśli 
 *   tak, usuwa z planszy wszystkie klocki tej grupy;
 * - porządkuje planszę, przesuwając pełne pola w dół i niepuste kolumny
 *   w lewo;
 * - pisze wynik.
 *
 * autor: Bogna Pawlus
 * data: 14 grudnia 2022
 * wersja: 1.2 
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

/* postać planszy */

/**
 * Liczba wierszy planszy.
 */
#ifndef WIERSZE
#define WIERSZE 10
#endif

/**
 * Liczba kolumn planszy.
 */
#ifndef KOLUMNY
#define KOLUMNY 15
#endif

/**
 * Liczba rodzajów klocków.
 */
#ifndef RODZAJE
#define RODZAJE 4
#endif

/* Wczytywanie planszy: */
void wczytaj(char plansza[][KOLUMNY]) {
    for (int w = 0; w < WIERSZE; w++) {
    	for (int k = 0; k < KOLUMNY; k++) {
    		int s = scanf(" %c", &plansza[w][k]);
    		assert(s == 1);
    	}
    }
}

/* Usuwanie klocków: */

/**
 * Funkcja sprawdzająca czy w tym ruchu będziemy cokolwiek usuwać. 
 * Czyli czy nie wybrano spacji i czy nie istnieje conajmniej 1 sąsiad z 
 * tą samą wartością
 */
int czy_istnieje_sasiad(char plansza[][KOLUMNY], int w, int k, char wartosc) {
	if (wartosc == '.') return 0;
	if (w - 1 >= 0 && plansza[w-1][k] == wartosc) return 1;
	if (k + 1 < KOLUMNY && plansza[w][k + 1] == wartosc) return 1;
	if (w + 1 < WIERSZE && plansza[w + 1][k] == wartosc) return 1;
	if (k - 1 >= 0 && plansza[w][k-1] == wartosc) return 1;
	return 0;
}

/**
 * Funkcja pomocnicza, która w każdym kroku rekurencji zamienia na spację
 * wszystkich niezamienionych jeszcze niepustych sąsiadów pola (w, k)
 * Na końcu wszystkie usunięte klocki są zamienione na spacje
 */
void usun_krzyzyk(char plansza[][KOLUMNY], int w, int k, char wartosc) {
	//zakładamy, że w (w, k) już jest spacja
	
	//usuwanie (w - 1, k)
	if (w - 1 >= 0 && plansza[w-1][k] == wartosc) {
		plansza[w - 1][k] = ' ';
		usun_krzyzyk(plansza, w - 1, k, wartosc);
	}
	
	//usuwanie (w, k + 1)
	if (k + 1 < KOLUMNY && plansza[w][k + 1] == wartosc) {
		plansza[w][k + 1] = ' ';
		usun_krzyzyk(plansza, w, k + 1, wartosc);
	}
	
	//usuwanie (w + 1, k)
	if (w + 1 < WIERSZE && plansza[w + 1][k] == wartosc) {
		plansza[w + 1][k] = ' ';
		usun_krzyzyk(plansza, w + 1, k, wartosc);
	}
	
	//usuwanie (w, k - 1)
	if (k - 1 >= 0 && plansza[w][k-1] == wartosc) {
		plansza[w][k - 1] = ' ';
		usun_krzyzyk(plansza, w, k - 1, wartosc);
	}
}

/**
 * Główna funkcja odpowiadająca za usuwanie pól.
 */
void usun(char plansza[][KOLUMNY], int w, int k, char wartosc) {
	if (czy_istnieje_sasiad(plansza, w, k, wartosc)) {
		plansza[w][k] = ' ';
		usun_krzyzyk(plansza, w, k, wartosc);
	}
}

/* Usuwanie klocków: */

/**
 * Zamienia dwie liczby
 */
void zamien_pola(char *x, char *y) {
	char t = *x;
	*x = *y;
	*y = t;
}

/**
 * Funkcja obniżająca kolumnę k po usuniętych przez spację polach
 */
void kolumna_w_dol(char plansza[][KOLUMNY], int k) {
	int a = WIERSZE - 1; 
	int b = WIERSZE - 1;

	while (b >= 0 && a >= 0 && plansza[a][k] != '.') {
		while (a >= 0 && plansza[a][k] != '.' && plansza[a][k] != ' ') a--;
		
		if (a >= 0 && plansza[a][k] != '.') {
			b = a;
			while (b >= 0 && plansza[b][k] == ' ') b--;
			
			if (b >= 0 && plansza[b][k] != '.') {
				zamien_pola(&plansza[a][k], &plansza[b][k]);
			}
			else { //chcemy mieć jeszcze spację, przy kol_w_l bedziemy kropkowac
				for (int i = a; i > b; i--) {
					plansza[i][k] = '.';
				}			
			}
		}
	}	
}

/**
 * Zamienia dwie kolumny nr a i nr b.
 */
void zamien_kolumny(char plansza[][KOLUMNY], int a, int b) {
	for (int i = WIERSZE - 1; i >= 0; i--) {
		zamien_pola(&plansza[i][a], &plansza[i][b]); 
	}
}

/**
 * Funkcja przesuwająca wszystkie niepuste kolumny w lewo.
 */
void kolumny_w_lewo(char plansza[][KOLUMNY]) {
	int a = 0;
	int b = 0;
	
	while (b < KOLUMNY && a < KOLUMNY) {
		while (a < KOLUMNY && plansza[WIERSZE - 1][a] != '.') a++;
		
		if (a < KOLUMNY) {
			b = a;
			while (b < KOLUMNY && plansza[WIERSZE - 1][b] == '.') b++;
			
			if (b < KOLUMNY) {
				zamien_kolumny(plansza, a, b);
			}
		}
	}
}

/* Wypisywanie wyniku: */
void wypisz(char plansza[][KOLUMNY]) {
    for (int w = 0; w < WIERSZE; w++) {
    	for (int k = 0; k < KOLUMNY; k++) {
    		printf("%c", plansza[w][k]);
    	}
    	printf("\n");
    }
}

/**
 * wczytyje i wypisuje planszę i parametry
 */
int main(int argc, char  *argv[]) {
    assert(argc == 3);
    char plansza[WIERSZE][KOLUMNY];
    wczytaj(plansza);
   
    //tu jest miejsce na zmianę planszy:
    int wusun = atoi(argv[1]);
    int kusun = atoi(argv[2]);
    char wartosc = plansza[wusun][kusun];
    usun(plansza, wusun, kusun, wartosc);
    for (int j = 0; j < KOLUMNY; j++) {
    	kolumna_w_dol(plansza, j);
    }
    kolumny_w_lewo(plansza);
    
    wypisz(plansza);

    return 0;
} 

