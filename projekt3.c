/**                                                                  
 * "Gra w życie" - zadanie zaliczeniowe 3
 *
 * Program symulujący grę w życie.
 *
 * Kompilacja poleceniem:
 *
 * gcc @opcje -DWIERSZE=22 -DKOLUMNY=80 zadanie3.c -o zadanie3
 *
 * Program wczytuje żywe komórki a potem rozpoznaje polecenia:
 * - zakończenia pracy -> '.',
 * - obliczenia N-tej generacji -> 'N',
 * - obliczenia następnej generacji -> ''
 * - zrzutu stanu aktualnej generacji -> '0'
 * - przesunięcia okna do innego rogu (w, k)-> 'w k'
 *
 * autor: Bogna Pawlus
 * data: 9 stycznia 2023
 * wersja: 1
 */

#include <limits.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/* 0. Postać planszy */

/**
 * Liczba wierszy planszy.
 */
#ifndef WIERSZE
#define WIERSZE 22
#endif

/**
 * Liczba kolumn planszy.
 */
#ifndef KOLUMNY
#define KOLUMNY 80
#endif

/* 1. Używane struktury */

/**
 * Typ Tlista
 * Trzymamy w nim żywe (lub podejrzane, które być może się ożywią w kolejnym
 * ruchu) komórki z ustalonego wiersza
 *
 * Te pola zawierają
 * - 'int val' -> numery kolumn z (potencjalnie) żywą komórką, uporządkowane rosnąco
 * - 'int czyzywe' -> liczba z {-1, 0, 1}. Komórka jest żywa gdy czyzywe == 1
 * - 'n_sasiadow' -> liczba sąsiadujących żywych komórek
 * - 'lista *next' -> wskaźnik do następnej kolumny z potencjalnie żywym polem
 *    w naszym wierszu
 * - 'lista *prev' -> wskaźnik do poprzedniej kolumny z potencjalnie żywym polem
 *    w naszym wierszu 
 */
struct lista {
	int val;
	int czyzywe;
	int n_sasiadow;
	struct lista *next;
	struct lista *prev;
};

typedef struct lista Tlista;

/**
 * Typ Ttablica
 * Każde pole tego typu zawiera:
 * - numer wiersza 'int row' -> wiersz, w którym jest conajmniej 1 żywa (lub 
 *   podejrzana, które być może się ożywi w kolejnym ruchu) komórka
 * - wskaźnik curr_row na typ Tlista z (potencjalnie) żywymi komórkami
 *   w wierszu 'row'
 * - 'tablica *next_row*' -> wskaźnik na następny wiersz z potencjalnie żywymi
 * - 'tablica *prev_row*' -> wskaźnik na poprzedni wiersz z potencjalnie żywymi
 * Początek Ttablicy to pole zawierające numer najmniejszego wiersza z 'potencjalnie'
 * żywą komórką
 */
struct tablica {
	int row; //który wiersz
	struct lista *curr_row;
	struct tablica *next_row;
	struct tablica *prev_row;	
};

typedef struct tablica Ttablica;

/* 2.1 Implemenacja podstawowych działań na tych strukturach --> typ TLISTA */

/**
 * Wypisuje całą listę
 */
void Wyswietl(Tlista* pocz) {
    Tlista* w = pocz;
    while (w) {
        printf(" %d", w->val);
        w = w->next;
    }
    printf("\n");
}

/**
 * Usuwa całą listę
 */
void usun(Tlista* pocz) {
	Tlista* l = pocz;
	while (l) {
		Tlista* nast = l->next;
		free(l);
		l = nast;
	}
}

/**
 * Dodaje nowe pole do Tlisty po prawej stronie od pola 'pole'
 * 'int wartosc' -> numer kolumny
 * 'int czyzyje' -> liczba z {-1, 0, 1}
 * Zwraca wskaźnik na nowododane pole
 */
Tlista* dodaj_po_prawej(Tlista* pole, int wartosc, int czyzyje) {
	Tlista* do_dodania = malloc(sizeof(Tlista));
	Tlista* poprz = pole;
	Tlista* nast = pole->next;
	
	do_dodania->val = wartosc;
	do_dodania->czyzywe = czyzyje;
	do_dodania->prev = poprz;
	do_dodania->next = nast;
	
	poprz->next = do_dodania;
	
	if (nast != NULL) nast->prev = do_dodania;
	
	return do_dodania;
}

/**
 * Dodaje nowe pole do Tlisty po lewej stronie od pola 'pole'
 * 'int wartosc' -> numer kolumny
 * 'int czyzyje' -> liczba z {-1, 0, 1}
 * Zwraca wskaźnik na nowododane pole
 */
Tlista* dodaj_po_lewej(Tlista* pole, int wartosc, int czyzyje) {
	Tlista* do_dodania = malloc(sizeof(Tlista));
	Tlista* poprz = pole->prev;
	Tlista* nast = pole;
	
	do_dodania->val = wartosc;
	do_dodania->czyzywe = czyzyje;
	do_dodania->prev = poprz;
	do_dodania->next = nast;
	
	nast->prev = do_dodania;
	
	if (poprz != NULL) poprz->next = do_dodania;
	
	return do_dodania;
}

/* 2.1 Implemenacja podstawowych działań na tych strukturach --> typ TTABLICA */

/**
 * Wypisuje całą Ttablicę
 */
void wyswietl_Ttablica(Ttablica* pocz) {
	if (pocz != NULL) {
		Ttablica* t = pocz;
		while (t) {
			printf("/%d", t->row);
			Wyswietl(t->curr_row);
			t = t->next_row;
		}
		printf("/\n");
	}
	else printf("/\n");
}

/**
 * Usuwa całą Ttablicę
 */
void usun_Ttablica(Ttablica* pocz) {
	Ttablica* t = pocz;
	while (t) {
		Ttablica* nast = t->next_row;
		usun(t->curr_row);
		free(t);
		t = nast;
	}
}

/**
 * Tworzy pole Ttablica z 1 (potencjalnie) żywą komorką poniżej wiersza 'w'
 * Zwraca wskaznik na nowododany wiersz
 * - 'Ttablica* w -> pole, zaraz pod krórym dokładamy nowe
 * - 'int nr_nowego_wiersza' -> wiersz, który teraz uzupełniamy
 * - 'int kol' -> pierwsza z lewej kolumna z potencjalnie żywym, której numer
 *    dodajemy do wiersza 'w'
 * - 'int czyzyje' -> wartość czyzyje dla pierwszego pola z potencjalnie żywym
 *    w rozważanym wierszu w
 */
Ttablica* stworz_Ttab_pod(Ttablica* w, int nr_nowego_wiersza, int kol, int czyzyje) {
	Ttablica* do_dodania = malloc(sizeof(Ttablica));
	Ttablica* poprz = w;
	Ttablica* nast = w->next_row;
	
	do_dodania->row = nr_nowego_wiersza;
	Tlista* l = malloc(sizeof(Tlista));
	l->czyzywe = czyzyje;
	l->val = kol;
	l->prev = NULL;
	l->next = NULL;
	do_dodania->curr_row = l;
	do_dodania->prev_row = poprz;
	do_dodania->next_row = nast;
	
	poprz->next_row = do_dodania;
	if (nast != NULL) nast->prev_row = do_dodania;
	
	return do_dodania;
}

/**
 * Tworzy pole Ttablica z 1 (potencjalnie) żywą komorką powyżej wiersza 'w'
 * Zwraca wskaznik na nowododany wiersz
 * - 'Ttablica* w -> pole, zaraz nad którym dokładamy nowe
 * - 'int nr_nowego_wiersza' -> wiersz, który teraz uzupełniamy
 * - 'int kol' -> pierwsza z lewej kolumna z potencjalnie żywym, której numer
 *    dodajemy do wiersza 'w'
 * - 'int czyzyje' -> wartość czyzyje dla pierwszego pola z potencjalnie żywym
 *    w rozważanym wierszu w
 */
Ttablica* stworz_Ttab_nad(Ttablica* w, int nr_nowego_wiersza, int kol, int czyzyje) {
	Ttablica* do_dodania = malloc(sizeof(Ttablica));
	Ttablica* poprz = w->prev_row;
	Ttablica* nast = w;
	
	do_dodania->row = nr_nowego_wiersza;
	Tlista* l = malloc(sizeof(Tlista));
	l->czyzywe = czyzyje;
	l->val = kol;
	l->prev = NULL;
	l->next = NULL;
	do_dodania->curr_row = l;
	do_dodania->prev_row = poprz;
	do_dodania->next_row = nast;
	
	nast->prev_row = do_dodania;
	if (poprz != NULL) poprz->next_row = do_dodania;
	
	return do_dodania;
}

/* 3. Uzupełnianie planszy na samym początku */

/**
 * Zwiększa rozmiar napisu
 */
int wiecej(int n) {
    assert(n <= (INT_MAX - 2) / 3 * 2);
    return n / 2 * 3 + 2;
}

/**
 * Wczytuje z wejścia jedynie żywe komórki (pomija pierwsze '/')
 */
void wczytaj_zywe(char **zywe, int *n) { //*n -> ile znaków
    char *wynik = NULL;
    int rozmiar = 0;
    int i = 0;
    int znak = getchar(); //aktualny znak
    int poprzedni = 'x';
    
    while (!(poprzedni == '/' && znak == '\n')) {
    	poprzedni = znak;
    	znak = getchar(); 
        if (i == rozmiar) {
            rozmiar = wiecej(rozmiar);
            wynik = realloc(wynik, (size_t) rozmiar * sizeof *wynik);
            assert(wynik != NULL);
        }
        wynik[i] = (char) znak;
        i++;
    }
    
    *zywe = wynik;
    *n = i;	
}

/**
 * Liczy, ile w napisie 'zywe' znaleziono najwięcej żywych pól w jednej linii
 */
int ile_najwiecej_zywych_w_lini(char *zywe, int ile) {
	int max_spacje = 0;
	int akt_spacje = 0;
	for (int i = 0; i < ile; i++) {
		if (zywe[i] == ' ') {
			akt_spacje++;
		}
		else if (zywe[i] == '/') {
			if (akt_spacje > max_spacje) max_spacje = akt_spacje;
			akt_spacje = 0;
		}
	}
	return max_spacje;
}

/**
 * Przepisuje 1 ustalony wiersz do tablicy tab[] o rozmiarze
 * N_tab = ile_najwiecej_zywych_w_linii
 * rozpoczyna przepisywanie od znaku zywe[*start] (i zwiększa *start)
 * konczy gdy znajdzie '/'
 */
void linia_do_tablicy(char *zywe, int *start, int *nr_wiersza, int tab[], int *N_tab) {
	if (zywe[*start] == '/') *start += 1;
	*nr_wiersza = atoi(zywe + *start); 
	while (zywe[*start] != ' ') *start += 1; 
	
	*N_tab = 0;
	while (zywe[*start] != '/') {
		*start += 1; 
		tab[*N_tab] = atoi(zywe + *start);
		*N_tab += 1;
		while (zywe[*start] != ' ' && zywe[*start] != '/') *start += 1;
	}
} 

/**
 * Zamienia dany wiersz z numerami żywych kolumn z tablicy na strukture Tlista
 * Zwraca początek listy
 * Ustawia czyzywe na 1
 * Jest potrzebne tylko raz na początku
 */
Tlista* Stworz_z_tablicy(int A[], int N) {
	if (N > 0) {
		Tlista* do_zwrocenia;
		Tlista* poprz = NULL;
		for (int i = 0; i < N; i++) {
			Tlista* akt = malloc(sizeof(Tlista));
			akt->val = A[i];
			akt->czyzywe = 1;
			akt->prev = poprz;
			if (i != 0) poprz->next = akt;
			else do_zwrocenia = akt;
			poprz = akt;
		}
		poprz->next = NULL;
		return do_zwrocenia;
	}
	else {
		return NULL;
	}
}

/**
 * GŁÓWNA FUNKCJA
 * Zamienia napis *zywe na strukturę Ttablica
 */
Ttablica* tablica_poczatek(char *zywe, int ile) {
	int start = 0;
	int max_linia = ile_najwiecej_zywych_w_lini(zywe, ile);
	Ttablica* do_zwrocenia = NULL;
	Ttablica* poprz = NULL;
	int *tab = malloc((size_t)max_linia * sizeof(int));
	
	int ind = 0;
	while (start < ile - 2) {
		int nr_wiersza;
		int rozmiar_tab; //ile liczb w aktualnym wierszu
		linia_do_tablicy(zywe, &start, &nr_wiersza, tab, &rozmiar_tab);
		Tlista* linia = Stworz_z_tablicy(tab, rozmiar_tab);
		
		//uzupelniamy pole z numerem wiersza
		Ttablica *t = malloc(sizeof(Ttablica));
		t->row = nr_wiersza;
		t->curr_row = linia;
		t->prev_row = poprz;
		if (ind != 0) poprz->next_row = t;
		else do_zwrocenia = t; 
		poprz = t;
		ind++;
	}
	poprz->next_row = NULL;
	free(tab);
	return do_zwrocenia;
}


/* 4. TWORZENIE OTOCZKI */
/**
 * Zrobimy w Ttablicy otoczkę żywych, do której dodamy komórki nieżywe, które mają co 
 * najmniej jednego sąsiada z jakąkolwiek aktualnie żywą komórką. Niektóre z tych 
 * komórek mogą się potencjalnie ożywić w kolejnym ruchu. Te, które się nieożywią 
 * usuniemy.
 */

/**
 * Tworzy otoczkę wiersza
 * Dodaje do Tlisty pola w ustalonym wierszu lpocz, które w tym samym wierszu mają co
 * najmniej jednego sąsiada. W nowododanych komórkach ustawiamy czyzyje = 0.
 */
Tlista* otoczka_wiersza(Tlista* lpocz) {
	Tlista* l = lpocz;
	l = dodaj_po_lewej(l, l->val - 1, 0);
	Tlista* do_zwrocenia = l;
	
	while (l != NULL) {
		while (l->next != NULL && l->next->val == l->val + 1) l = l->next;
		
		if (l->next == NULL) {
			dodaj_po_prawej(l, l->val + 1, 0);
			l = NULL;
		}
		else if (l->next->val == l->val + 2) {
			dodaj_po_prawej(l, l->val + 1, 0);
			l = l->next->next;
		}
		else {
			Tlista* stary_next = l->next;
			dodaj_po_prawej(l, l->val + 1, 0);
			dodaj_po_lewej(stary_next, stary_next->val - 1, 0);
			l = stary_next;
		}
	}
	return do_zwrocenia;
}

/**
 * Funkcja dodająca komórki mające sąsiada z czyzyje == 1 jedynie z dołu
 * które jeszcze nie dodane przez otoczka_wiersza.
 * Używamy gdy znajdziemy potencjalnie żywą w wierszy nieistniejącym w Ttablicy
 * Przepisuje nad wierszem t, wiersz z tymi samymi kolumnami, co t.
 * Zakładamy, że na t zadziałaliśmy już przez otoczka_wiersza i mogą w t wystąpić
 * pola z czyzyje = 1 i czyzyje = 0.
 * Ustawia czyzyje = -1.
 */
Ttablica* dodaj_wiersz_z_gory(Ttablica* t) {
	Tlista* l = t->curr_row;
	while (l->czyzywe == -1) l = l->next;
	
	Ttablica* t1 = stworz_Ttab_nad(t, t->row - 1, l->val, -1);
	l = l->next;	
	Tlista* l1 = t1->curr_row; //tu dodajemy
	while (l) {
		if (l->czyzywe != -1) l1 = dodaj_po_prawej(l1, l->val, -1);
		l = l->next;
	}
	return t1;
}

/**
 * Funkcja dodająca komórki mające sąsiada z czyzyje == 1 jedynie z góry
 * które jeszcze nie dodane przez otoczka_wiersza.
 * Używamy gdy znajdziemy potencjalnie żywą w wierszy nieistniejącym w Ttablicy
 * Przepisuje pod wierszem t, wiersz z tymi samymi kolumnami, co t.
 * Zakładamy, że na t zadziałaliśmy już przez otoczka_wiersza i mogą w t wystąpić
 * pola z czyzyje = 1 i czyzyje = 0.
 * Ustawia czyzyje = -1.
 */
Ttablica* dodaj_wiersz_z_dolu(Ttablica* t) {
	Tlista* l = t->curr_row;
	while (l->czyzywe == -1) l = l->next;
	
	Ttablica* t1 = stworz_Ttab_pod(t, t->row + 1, l->val, -1);
	l = l->next;
	Tlista* l1 = t1->curr_row;
	while (l) {
		if (l->czyzywe != -1) l1 = dodaj_po_prawej(l1, l->val, -1);
		l = l->next;
	}
	return t1;
}

/**
 * Funkcja dodająca do wiersza a pola z wiersza b, które być może w następnym
 * ruchu się ożywią, tzn. przepisuje do wiersza a pola z wiersza b, które w
 * wierszu a jeszcze nie istnieją
 * Zakładamy, że w a i b zadziałaliśmy już przez otoczka_wiersza i mogą wystąpić
 * w a i b pola z czyzyje = 1 i czyzyje = 0.
 * Ustawia czyzyje = -1. 
 */
Ttablica* polacz_z_wierszem(Ttablica* a, Ttablica* b) {
	Tlista* akt = a->curr_row;
	Tlista* pom = b->curr_row;
	
	//zwracanie
	while (pom != NULL && pom->czyzywe == -1) pom = pom->next;
	if (pom != NULL) {
		if (pom->val < akt->val) {
			akt = dodaj_po_lewej(akt, pom->val, -1);
			a->curr_row = akt;
			pom = pom->next;
		}

		while (pom) {
			while (pom != NULL && pom->czyzywe == -1) pom = pom->next;
			
			if (pom != NULL) {
				if (pom->val < akt->val) {
					akt = dodaj_po_lewej(akt, pom->val, -1);
					pom = pom->next;
				}
				else if (pom->val > akt->val) {
					if (akt->next != NULL) {
						akt = akt->next;
					}
					else {
						akt = dodaj_po_prawej(akt, pom->val, -1);
					}
				}
				else { //rowne;
					pom = pom->next;
				}
			}
		}	
	}
	return a;
}

/**
 * GŁÓWNA FUNKCJA
 * Tworzy otoczkę Ttablicy, tzn dodaje do Ttablicy wszystkie pola, które 
 * mają co najmniej 1 żywego sąsiada. 
 * zwraca początek Ttablicy
 */
Ttablica* zrob_otoczke(Ttablica* pocz) {
	Ttablica* t = pocz;	
	while (t) {
		t->curr_row = otoczka_wiersza(t->curr_row);
		t = t->next_row;
	}
	
	t = pocz;
	pocz = dodaj_wiersz_z_gory(t); //wskazuje na nowy wiersz
	
	while (t->next_row != NULL) {
		if (t->row + 1 == t->next_row->row) { //prev_row istnieje, bo t > 0
			t = polacz_z_wierszem(t, t->next_row);
			t->next_row = polacz_z_wierszem(t->next_row, t);
			t = t->next_row;
		}
		else if (t->row + 2 == t->next_row->row) {
			Ttablica* nowy = dodaj_wiersz_z_dolu(t);
			nowy = polacz_z_wierszem(nowy, nowy->next_row);
			t = nowy->next_row;
		}
		else {
			Ttablica* nastepny = t->next_row;
			t = dodaj_wiersz_z_dolu(t);
			t = dodaj_wiersz_z_gory(nastepny); 
			t = nastepny;
		}
	}
	t = dodaj_wiersz_z_dolu(t);
	return pocz;	
}

/* 5. LICZENIE SĄSIADÓW POTENCJALNIE ŻYWYCH PÓL */
/**
 * Po zrobieniu otoczki Ttablicy, chcemy zdecydować które pola nie są żywe i 
 * je usuwamy. Dlatego dla każdego pola z Ttablicy policzymy, ile ma żywych 
 * sąsiadów.
 */
 
/**
 * Liczymy ile pole akt z wiersza nr n ma sąsiadów z polem nast
 * pole nast znajduje się w (niepustym) wierszu postencjalnie żywych n - 1 lub n + 1
 * zakładamy, że |akt->val - nast->val| <= 1
 * zwiększamy *licz o liczbę nowych sąsiadów
 */
void sasiedzi_z_poprzedniego_wiersza(Tlista* akt, Tlista* nast, int *licz) {
	if (nast->val == akt->val + 1) { // 1 do sprawdzenia
		if (nast->czyzywe == 1) *licz += 1;
	}
	else if (nast->val == akt->val) { //2 do sprawdzenia
		if (nast->czyzywe == 1) *licz += 1;
		if (nast->next != NULL && nast->next->val == akt->val + 1) {
			if (nast->next->czyzywe == 1) *licz += 1;
		}
	}	
	else if (nast->val == akt->val - 1) { // 3 do sprawdzenia
		if (nast->czyzywe == 1) *licz += 1;
		
		if (nast->next != NULL) {
			if (nast->next->val == akt->val) { //2 do sprawdzenia
				if (nast->next->czyzywe == 1) *licz += 1;
				if (nast->next->next != NULL) {
					if (nast->next->next->val == akt->val + 1) {
						if (nast->next->next->czyzywe == 1) *licz += 1;
					}
				}
			}
			else if (nast->next->val == akt->val + 1) { //1 do sprawdzenia
				if (nast->next->czyzywe == 1) *licz += 1;
			}
		}
	}
}
 
/**
 * Funkcja licząca ile żywych sąsiadów ma ustalone pole Tlista* akt.
 * Załóżmy, że wiersze poprz, akt, nast1 mają numery n, n + 1, n + 2 odpowiednio.
 * Gdy w Ttablica nie istnieją wiersze z potencjalnie żywymi o numerach 
 * n lub n+2 możemy wziąć NULL za listę poprz lub nast1.
 * Zakładamy, że akt != NULL.
 */
int ile_pole_ma_sasiadow(Tlista* poprz, Tlista* akt, Tlista* nast1) {
	int licz = 0;
	
	if (akt->prev != NULL && akt->prev->czyzywe == 1) licz++;
	if (akt->next != NULL && akt->next->czyzywe == 1) licz++;
	 
	//liczymy sąsiadów z następnej linii
	if (nast1 != NULL) {
		sasiedzi_z_poprzedniego_wiersza(akt, nast1, &licz);
	}
	
	//liczymy sasiadów z poprzedniej linii
	if (poprz != NULL) {
		sasiedzi_z_poprzedniego_wiersza(akt, poprz, &licz);
	}	
	
	return licz;
}

/**
 * Liczy sąsiadów dla listy z wiersza t
 */
void sasiedzi_wiersz(Ttablica* t) {
	if (t->prev_row != NULL && t->next_row != NULL) {
		Tlista* poprz = t->prev_row->curr_row;
		Tlista* akt = t->curr_row;
		Tlista* nast = t->next_row->curr_row;
		
		while (akt != NULL) {
			while (nast != NULL && nast->val < akt->val - 1) nast = nast->next;
			while (poprz != NULL && poprz->val < akt->val - 1) poprz = poprz->next;
			akt->n_sasiadow = ile_pole_ma_sasiadow(poprz, akt, nast);
			akt = akt->next;
		}
	}
	else if (t->prev_row == NULL) {
		Tlista* poprz = NULL;
		Tlista* akt = t->curr_row;
		Tlista* nast = t->next_row->curr_row;
		
		while (akt != NULL) {
			while (nast != NULL && nast->val < akt->val - 1) nast = nast->next;
			akt->n_sasiadow = ile_pole_ma_sasiadow(poprz, akt, nast);
			akt = akt->next;
		}	
	}
	else {
		Tlista* poprz = t->prev_row->curr_row;
		Tlista* akt = t->curr_row;
		Tlista* nast = NULL;
		
		while (akt != NULL) {
			while (poprz != NULL && poprz->val < akt->val - 1) poprz = poprz->next;
			akt->n_sasiadow = ile_pole_ma_sasiadow(poprz, akt, nast);
			akt = akt->next;
		}	
	}
}

/**
 * GŁÓWNA FUNKCJA
 * Wypełnia liczbę sąsiadów (t->n_sasiadów) dla każdego pola Tlista w stukturze
 * Ttablica
 */
Ttablica* wypelnij_sasiadow(Ttablica* pocz) {
	Ttablica* t = pocz;
	while (t) {
		sasiedzi_wiersz(t);
		t = t->next_row;
	}	
	return pocz;
}

/* 6. TWORZENIE GOTOWEJ TABLICY PO USUNIĘCIU ZŁYCH Z OTOCZKI */
/**
 * Usuwamy z Ttablicy pola nieżywe po kolejnym ruchu
 * Bierzemy pod uwagę lista->czyzyje i lista->n_sasiadow
 * (pole żyje <=> czyzyje == 1)
 */

/**
 * usuwanie wskazanego pola T_tablica przy zalozeniu, ze t->curr_row == NULL
 * zwraca wskaznik na początek tablicy
 */
Ttablica* usun_pole_z_Ttablicy(Ttablica* pole, Ttablica* pocz) {
	Ttablica* poprz = pole->prev_row;
	Ttablica* nast = pole->next_row;
	free(pole);
	
	if (poprz != NULL) poprz->next_row = nast;
	if (nast != NULL) nast->prev_row = poprz;
	
	if (pole == pocz) return nast;
	else return pocz;
}

/**
 * Usuwa wskazane pole z Tlista
 * Zwraca wskaźnik na początek listy
 */
Tlista* usun_pole_z_listy(Tlista* pole, Tlista* pocz) {
	Tlista* poprz = pole->prev;
	Tlista* nast = pole->next;
	free(pole);
	
	if (poprz != NULL) poprz->next = nast;
	if (nast != NULL) nast->prev = poprz;
	
	if (pocz == pole) return nast;
	else return pocz;
}

/**
 * Stwierdza, czy po ruchu pole będzie martwe czy żywe.
 */
int czy_pole_przezyje(int czy_zywe, int n_sasiadow) {
	if (czy_zywe == 1) {
		if (n_sasiadow == 2 || n_sasiadow == 3) return 1;
		else return 0;
	}
	else {
		if (n_sasiadow == 3) return 1;
		else return 0;
	}
}

/**
 * Usuwa z Ttablicy wszystkie pola, które będą martwe po ruchu
 */
Ttablica* pola_zywe_po_ruchu(Ttablica* pocz) {
	Ttablica* t = pocz;
	
	while (t) {
		Tlista* l = t->curr_row;
		while (l) {
			if (czy_pole_przezyje(l->czyzywe, l->n_sasiadow) == 0) {
				Tlista* nast = l->next;
				t->curr_row = usun_pole_z_listy(l, t->curr_row);
				l = nast;
			}
			else {
				l = l->next;
			}
		}
		
		if (t->curr_row != NULL) {
			t = t->next_row;
		}
		else {
			Ttablica* nastT = t->next_row;
			pocz = usun_pole_z_Ttablicy(t, pocz);
			t = nastT;
		}
	}
	return pocz;
}

/**
 * Po wykonaniu pola_zywe_po_ruchu ustawi na wszystkich polach Ttablicy
 * czyzywe = 1
 */
Ttablica* ustaw_zywe_na_1(Ttablica* pocz) {
	if (pocz != NULL) {
		Ttablica* t = pocz;
		while (t) {
			Tlista* l = t->curr_row;
			while (l) {
				l->czyzywe = 1;
				l = l->next;
			}
			t = t->next_row;
		}
	}
	return pocz;
} 

/* 7. WYŚWIETLANIE EKRANU */

/**
 * minimum
 */
int minf(int a, int b) {
	if (a < b) return a;
	else return b;
}

/**
 * Wyświetla 1 wiersz na ekranie
 */
void zrzut_wiersza(Tlista* l, int k) {//k-startowa kolumna
	while (l != NULL && l->val < k) l = l->next;
	for (int i = k; i < KOLUMNY + k; i++) {
		if (l == NULL || l->val > i) {
			printf(".");
		}
		else {
			printf("0");
			l = l->next;
		}
	}
}

/**
 * Wypisuje cały ekran, zaczynając od współrzędniej (w, k)
 * na samym początku w = k = 1
 */
void zrzut_ekranu(Ttablica* pocz, int w, int k) {
	Ttablica* t = pocz;
	
	while (t != NULL && t->row < w) t = t->next_row;
	for (int i = w; i < WIERSZE + w; i++) {
		if (t == NULL || t->row > i) {
			for (int s = 0; s < KOLUMNY; s++) printf(".");
		} 
		else {
			zrzut_wiersza(t->curr_row, k);
			t = t->next_row;
		}
		printf("\n");
	}
	
	for (int i = 0; i < KOLUMNY; i++) printf("=");
	printf("\n");
}

/* 8. WCZYTYWANIE RUCHÓW */

/**
 * wczytuje wiersz zawierający ruch i dopisuje na końcu ','
 */
void czytaj_1_wiersz(char **a, int *n) {
	char *wynik = NULL;
	int rozmiar = 1;
	wynik = realloc(wynik, (size_t) rozmiar * sizeof *wynik);
	int i;
	int znak;
	for (i = 0; ((znak = getchar()) != '\n') && (znak != EOF); ++i) {
		if (i + 1 >= rozmiar) {
		rozmiar = wiecej(rozmiar);
		wynik = realloc(wynik, (size_t) rozmiar * sizeof *wynik);
		assert(wynik != NULL);
		}
		wynik[i] = (char) znak;
	}
	wynik[i] = ','; 
	i++;
    
	*a = wynik;
	*n = i;
}

/**
 * Sprawdza czy napis znaki zawiera spację
 */
int jest_spacja(char *znaki, int ile) {
	int i = 0;
	while (i < ile && znaki[i] != ' ') i++;
	if (ile > i) return 1;
	else return 0;
}

/**
 * Liczy nową generację komórek
 */
Ttablica* nowa_generacja(Ttablica* tablica_zywych) {
	if (tablica_zywych == NULL) {
		return NULL;
	}
	else {
		tablica_zywych = zrob_otoczke(tablica_zywych);
		tablica_zywych = wypelnij_sasiadow(tablica_zywych);
		tablica_zywych = pola_zywe_po_ruchu(tablica_zywych);
		tablica_zywych = ustaw_zywe_na_1(tablica_zywych);
		return tablica_zywych;
	}
}

/**
 * Pętla z kolejnymi poleceniami:
 */
int main(void) {
	int ile;
	char *znaki;
	wczytaj_zywe(&znaki, &ile);
	Ttablica* tablica_zywych = tablica_poczatek(znaki, ile);
	char *ruch;
	int end = 1;
	int w_start = 1;
	int k_start = 1;
	zrzut_ekranu(tablica_zywych, w_start, k_start);
	while (end) {
		czytaj_1_wiersz(&ruch, &ile);
    		if (ruch[0] == ',') { //nowa generacja
			tablica_zywych = nowa_generacja(tablica_zywych);
			zrzut_ekranu(tablica_zywych, w_start, k_start);
		}
		else if (ruch[0] == '.') { //koniec
			end = 0;
		}
		else if (jest_spacja(ruch, ile)) { //zmiana poczatku na w_start, k_start
			w_start = atoi(ruch);
			int ind = 1;
			while (ruch[ind] != ' ') ind++;
			k_start = atoi(ruch + ind);
			zrzut_ekranu(tablica_zywych, w_start, k_start);
		}
		else { // 1 liczba
			int N = atoi(ruch);
			if (N == 0) { //zrzut Ttablicy
				wyswietl_Ttablica(tablica_zywych);
				zrzut_ekranu(tablica_zywych, w_start, k_start);
			}
			else {
				for (int i = 0; i < N; i++) { //N dalszych generacji
					tablica_zywych = nowa_generacja(tablica_zywych);					
				}
				zrzut_ekranu(tablica_zywych, w_start, k_start);
			}
		}
		free(ruch);
	}	
	usun_Ttablica(tablica_zywych);
	free(znaki);
	return 0;
}
