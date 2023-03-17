/**
 * "Palindrom" - projekt zaliczeniowy 1
 * autor: Bogna Pawlus
 * aktualna wersja
 */

#include <stdio.h>

/* rozmiary */

/**
 * Domyślne wartości rozmiarów planszy i długości palindromu z możliwością zmiany.
 */
#ifndef WIERSZE
#define WIERSZE 8
#endif

#ifndef KOLUMNY
#define KOLUMNY 8
#endif

#ifndef DLUGOSC
#define DLUGOSC 5
#endif

/* Wypełnienie planszy */

/**
 * Funkcja wypełniająca pauzami tablicę planszy przed pierwszym ruchem
 */
void wypelnij(char a[][KOLUMNY]) {
    for (int iw = 0; iw < WIERSZE; iw++) {
        for (int ik = 0; ik < KOLUMNY; ik++) {
            a[iw][ik] = '-';
        }
    }    
}

/**
 * Funkcja wypisująca tablicę planszy, po dowolnym ruchu
 */
void pisz(char a[][KOLUMNY]) {
    for (int iw = 0; iw < WIERSZE; iw++) {
        for (int ik = 0; ik < KOLUMNY; ik++) {
            putchar(' ');
            putchar(a[iw][ik]);
        }
        putchar('\n');
    }
}

/**
 * Funkcja wypisująca litery oznaczające kolumny
 */
void alfabet() { //wypisuje oznaczenia kolumn
    for (char i = 'a'; i < KOLUMNY + 'a'; i++) {
        putchar(' ');
        putchar(i);
    }
    putchar('\n');
} 

/**
 * Funkcja zmieniająca zawartość tablicy
 * Tylko dla ruchów, w których nie konczymy gry, czyli kto == '1' lub kto == '2'
 */
void ruch(char a[][KOLUMNY], char kto, char nr_kol) {
    int kol = nr_kol - 'a'; //ruch w tej kolumnie
    int wie = WIERSZE - 1;

    while (a[wie][kol] != '-') wie--;

    a[wie][kol] = kto;
}

/* Wypełnienie planszy */

/**
 * Funkcja szukająca palindromów w wierszu o początku w polu (w, k)
 * Sprawdzi czy (w, k), (w, k + 1), (w, k + 2), ...(w, k + DLUGOSC - 1) to palindrom
 * Zakładamy, że k + DLUGOSC -1 < KOLUMNY
 */
int czy_palindrom_wierszowy(char a[][KOLUMNY], int w, int k) {
    int pocz = k;
    int kon = k + DLUGOSC - 1;

    while (pocz < kon) {
        if (a[w][pocz] != a[w][kon] || a[w][pocz] == '-') {
            return 0;
        }
        else {
            pocz++;
            kon--;
        }
    }
    if (a[w][pocz] == '-') return 0; //znaleziono w środku palindroma pauzę
    else return 1;
}

/**
 * Funkcja szukająca palindromów w kolumnie o początku w polu (w, k)
 * Sprawdzi czy (w, k), (w + 1, k), (w + 2, k), ..., (w + DLUGOSC - 1, k) to palindrom
 * Zakładamy, że w + DLUGOSC - 1 < WIERSZE
 */
int czy_palindrom_kolumnowy(char a[][KOLUMNY], int w, int k) {
    int pocz = w;
    int kon = w + DLUGOSC - 1;

    while (pocz < kon) {
        if (a[pocz][k] != a[kon][k] || a[pocz][k] == '-') {
            return 0;
        }
        else {
            pocz++;
            kon--;
        }
    }
    return 1; //w środku palindorma kolumnowego nie będzie pauzy 
}

/**
 * Funkcja szukająca palindormów ukośnych w dół
 * Sprawdzi czy (w, k), (w + 1, k + 1), ... (w + DLUGOSC - 1, k + DLUGOSC - 1) to palindorm
 * Zakładamy, że w + DLUGOSC - 1 < WIERSZE   i   k + DLUGOSC - 1 < KOLUMNY
 */
int czy_palindrom_przekatny_dol(char a[][KOLUMNY], int w, int k) {
    int poczw = w;
    int poczk = k;
    int konw = w + DLUGOSC - 1;
    int konk = k + DLUGOSC - 1;

    while (poczw < konw) {
        if (a[poczw][poczk] != a[konw][konk] || a[poczw][poczk] == '-') {
            return 0;
        }
        else {
            poczw++;
            poczk++;
            konw--;
            konk--;
        }
    }
    if (a[poczw][poczk] == '-') return 0; //znaleziono w środku palindroma pauzę
    else return 1;
}

/**
 * Funkcja szukająca palindromów ukośnych w górę
 * Sprawdzi czy w (w, k), (w + 1, k - 1), ... (w + DLUGOSC - 1, k - DLUGOSC + 1) to palindorm
 * Zakładamy, że w + DLUGOSC - 1 < WIERSZE   i   k - DLUGOSC + 1 >= 0
 */
int czy_palindrom_przekatny_gora(char a[][KOLUMNY], int w, int k) {
    int poczw = w;
    int poczk = k;
    int konw = w + DLUGOSC - 1;
    int konk = k - DLUGOSC + 1;

    while (poczw < konw) {
        if (a[poczw][poczk] != a[konw][konk] || a[poczw][poczk] == '-') {
            return 0;
        }
        else {
            poczw++;
            poczk--;
            konw--;
            konk++;
        }        
    }
    if (a[poczw][poczk] == '-') return 0; //znaleziono w środku palindroma pauzę
    else return 1;
}


/**
 * Główna funkcja szukająca wszystkich palindromów w tablicy po każdym ruchu
 * Wybiera wszystkie możliwe początki (wi, ki) palindromów
 * i sprawdza czy te pola są początkami palindomów dowolnego z 4 typów
 */
int czy_jest_palindrom(char a[][KOLUMNY]) {
        for (int wi = 0; wi < WIERSZE; wi++) {
            for (int ki = 0; ki <= KOLUMNY - DLUGOSC; ki++) {
                if (czy_palindrom_wierszowy(a, wi, ki)) return 1;
            }
        }

        for (int wi = WIERSZE - DLUGOSC; wi >= 0; wi--) { //palindromów wierszowych szukamy od dołu
            for (int ki = 0; ki < KOLUMNY; ki++) {
                if (czy_palindrom_kolumnowy(a, wi, ki)) return 1;
            }
        }

        for (int wi = 0; wi <= WIERSZE - DLUGOSC; wi++) {
            for (int ki = 0; ki <= KOLUMNY - DLUGOSC; ki++) {
                if (czy_palindrom_przekatny_dol(a, wi, ki)) return 1;
            }
        }

        for (int wi = 0; wi <= WIERSZE - DLUGOSC; wi++) {
            for (int ki = DLUGOSC - 1; ki < KOLUMNY; ki++) {
                if (czy_palindrom_przekatny_gora(a, wi, ki)) return 1;
            }
        }

    return 0;   
}

int main(void) {
    char A[WIERSZE][KOLUMNY];
    wypelnij(A);
    pisz(A);
    alfabet();

    char kto = '1';
    int endp = 1;
    
    while(endp) {
        char nr_kol;
        printf("%c:\n", kto);
        int ok = scanf(" %c", &nr_kol); 
        
        if (ok && nr_kol != '.') {      
            ruch(A, kto, nr_kol);
            pisz(A);
            alfabet();

            if(czy_jest_palindrom(A)) {
                printf("%c!\n", kto);
                endp = 0;
            }
            else {
                if(kto == '1') kto = '2';
                else kto = '1';
            }
        }
        else {
            endp = 0;
        }
    }
    
    return 0;
}
