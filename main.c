#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	unsigned int idComanda;
	char* dataLansareComanda;
	char* dataLivrareProgramata;
	char* numeClient;
	float sumaPlata;
} Comanda;

typedef struct NodLS {
	Comanda inf;
	struct NodLS* next;
} NodLS;

typedef struct {
	NodLS** vect;
	int nrElem;
} HashTable;

char* copiereSir(const char* s) {
	char* r = (char*)malloc(strlen(s) + 1);
	strcpy(r, s);
	return r;
}

Comanda copiereComanda(Comanda c) {
	Comanda r = c;
	r.dataLansareComanda = copiereSir(c.dataLansareComanda);
	r.dataLivrareProgramata = copiereSir(c.dataLivrareProgramata);
	r.numeClient = copiereSir(c.numeClient);
	return r;
}

void eliberareComanda(Comanda* c) {
	free(c->dataLansareComanda);
	free(c->dataLivrareProgramata);
	free(c->numeClient);
}

int functieHash(HashTable tabela, unsigned int cheie) {
	return cheie % tabela.nrElem;
}

void inserareNodLS(NodLS** cap, Comanda c) {
	NodLS* nou = (NodLS*)malloc(sizeof(NodLS));
	nou->inf = copiereComanda(c);
	nou->next = NULL;
	if (*cap == NULL) {
		*cap = nou;
		return;
	}
	NodLS* t = *cap;
	while (t->next != NULL) {
		t = t->next;
	}
	t->next = nou;
}

void inserareHash(HashTable tabela, Comanda c) {
	inserareNodLS(&tabela.vect[functieHash(tabela, c.idComanda)], c);
}

HashTable creareTabelaGoala(int dim) {
	HashTable tabela;
	tabela.nrElem = dim;
	tabela.vect = (NodLS**)calloc(dim, sizeof(NodLS*));
	return tabela;
}

HashTable creareTabelaDinFisier(const char* numeFisier, int dim) {
	HashTable tabela = creareTabelaGoala(dim);
	FILE* f = fopen(numeFisier, "r");
	if (f == NULL) {
		printf("Fisierul %s nu a putut fi deschis.\n", numeFisier);
		return tabela;
	}
	Comanda c;
	char lansare[11], livrare[11], client[50];
	while (fscanf(f, "%u %10s %10s %49s %f", &c.idComanda, lansare, livrare, client, &c.sumaPlata) == 5) {
		c.dataLansareComanda = lansare;
		c.dataLivrareProgramata = livrare;
		c.numeClient = client;
		inserareHash(tabela, c);
	}
	fclose(f);
	return tabela;
}

void afisareComanda(Comanda c) {
	printf("  Comanda %u | lansata %s | livrare %s | client %-10s | %.2f lei\n",
		c.idComanda, c.dataLansareComanda, c.dataLivrareProgramata, c.numeClient, c.sumaPlata);
}

void traversareLS(NodLS* cap) {
	for (; cap != NULL; cap = cap->next) {
		afisareComanda(cap->inf);
	}
}

void traversareHash(HashTable tabela) {
	for (int i = 0; i < tabela.nrElem; i++) {
		if (tabela.vect[i] != NULL) {
			printf("Pozitia %d:\n", i);
			traversareLS(tabela.vect[i]);
		}
	}
}

int comenziLivrareLaData(HashTable tabela, const char* data) {
	int numar = 0;
	for (int i = 0; i < tabela.nrElem; i++) {
		for (NodLS* t = tabela.vect[i]; t != NULL; t = t->next) {
			if (strcmp(t->inf.dataLivrareProgramata, data) == 0) {
				numar++;
			}
		}
	}
	return numar;
}

int stergeComenziLivrate(HashTable tabela, const char* dataCurenta) {
	int sterse = 0;
	for (int i = 0; i < tabela.nrElem; i++) {
		NodLS* t = tabela.vect[i];
		NodLS* anterior = NULL;
		while (t != NULL) {
			NodLS* urm = t->next;
			if (strcmp(t->inf.dataLivrareProgramata, dataCurenta) < 0) {
				if (anterior == NULL) {
					tabela.vect[i] = urm;
				}
				else {
					anterior->next = urm;
				}
				eliberareComanda(&t->inf);
				free(t);
				sterse++;
			}
			else {
				anterior = t;
			}
			t = urm;
		}
	}
	return sterse;
}

NodLS* comenziClientInLista(HashTable tabela, const char* client) {
	NodLS* cap = NULL;
	for (int i = 0; i < tabela.nrElem; i++) {
		for (NodLS* t = tabela.vect[i]; t != NULL; t = t->next) {
			if (strcmp(t->inf.numeClient, client) == 0) {
				inserareNodLS(&cap, t->inf);
			}
		}
	}
	return cap;
}

void dezalocareLS(NodLS** cap) {
	while (*cap != NULL) {
		NodLS* urm = (*cap)->next;
		eliberareComanda(&(*cap)->inf);
		free(*cap);
		*cap = urm;
	}
}

void dezalocareHash(HashTable* tabela) {
	for (int i = 0; i < tabela->nrElem; i++) {
		dezalocareLS(&tabela->vect[i]);
	}
	free(tabela->vect);
	tabela->vect = NULL;
	tabela->nrElem = 0;
}

int main() {
	HashTable tabela = creareTabelaDinFisier("comenzi.txt", 7);
	printf("Tabela de dispersie (chaining):\n");
	traversareHash(tabela);

	char data[11] = "2025-03-20";
	printf("\nIntroduceti o data de livrare (AAAA-LL-ZZ): ");
	if (scanf("%10s", data) != 1) {
		strcpy(data, "2025-03-20");
	}
	printf("\nComenzi programate pentru livrare la %s: %d\n", data, comenziLivrareLaData(tabela, data));

	const char* azi = "2025-03-15";
	int sterse = stergeComenziLivrate(tabela, azi);
	printf("\nAu fost sterse %d comenzi livrate inainte de %s:\n", sterse, azi);
	traversareHash(tabela);

	NodLS* comenziClient = comenziClientInLista(tabela, "Ionescu");
	printf("\nComenzile clientului Ionescu (lista separata):\n");
	traversareLS(comenziClient);

	dezalocareLS(&comenziClient);
	dezalocareHash(&tabela);
	return 0;
}
