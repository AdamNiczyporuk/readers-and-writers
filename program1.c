// Czytelnicy i pisarze. Z czytelni korzysta na okrągło pewna ilość czytelników i pisarzy, przy czym jednocześnie może w niej znajdować się albo dowolna ilość czytelników, 
// albo jeden pisarz, albo nikt - nigdy inaczej. Problem ten ma trzy rozwiązania - z możliwością zagłodzenia pisarzy, 
// z możliwością zagłodzenia czytelników oraz wykluczające zagłodzenie. Napisać:

// a) dwa programy symulujące dwa różne rozwiązania tego problemu, bez korzystania ze zmiennych warunkowych [17 p], lub
// b) dwa programy symulujące dwa różne rozwiązania tego problemu, przy czym jeden z nich musi korzystać ze zmiennych warunkowych (condition variable). [27 p], lub
// c) trzy programy symulujące trzy różne rozwiązania tego problemu, przy czym przynajmniej jeden z nich musi korzystać ze zmiennych warunkowych [34 p].

// Ilość wątków pisarzy R i czytelników W można przekazać jako argumenty linii poleceń. Zarówno czytelnicy jak i pisarze wkrótce po opuszczeniu czytelni próbują znów się do niej dostać.
//  Program powinien wypisywać komunikaty według poniższego przykładu:

// ReaderQ: 11 WriterQ: 10 [in: R:0 W:1]
// Oznacza to, że w kolejce przed czytelnią czeka 10 pisarzy i 11 czytelników a sama czytelnia zajęta jest przez jednego pisarza. Komunikat należy wypisywać w momencie zmiany którejkolwiek z tych 


// Program symulujący problem czytelników i pisarzy z możliwością zagłodzenia pisarzy


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Deklaracja zmiennych globalnych
int R; // Liczba czytelników
int W; // Liczba pisarzy

int writers_counter = 0; // Całkowita liczba pisarzy
int readers_counter = 0; // Całkowita liczba czytelników
int reading = 0; // Liczba aktualnie czytających czytelników
int writing = 0; // Liczba aktualnie piszących pisarzy

// Inicjalizacja muteksów
pthread_mutex_t mutex;
pthread_mutex_t lock_writer;

// Funkcja konwertująca łańcuch znaków na liczbę całkowitą
int stringToInt(const char *str) {
    if (str == NULL || *str == '\0') {
        return -1; // Błąd: pusty łańcuch znaków
    }

    char *endptr;
    long int value = strtol(str, &endptr, 10);

    if (endptr == str || *endptr != '\0') {
        return -1; // Błąd: nieprawidłowa konwersja
    }
    
    return (int)value; // Zwrócenie wyniku konwersji
}

// Funkcja reprezentująca wątek czytelnika
void *reader(void *r) {
    while (1) {
        pthread_mutex_lock(&mutex); // Blokada dostępu do zmiennych współdzielonych

        if (reading == 0) {
            pthread_mutex_lock(&lock_writer); // Blokada dla pisarzy, gdy pierwszy czytelnik wchodzi
        }
        reading++; // Zwiększenie liczby czytających czytelników
        pthread_mutex_unlock(&mutex); // Odblokowanie dostępu do zmiennych współdzielonych

        // Wypisanie stanu kolejki i czytelni
        printf("ReaderQ: %i WriterQ: %i [in: R: %i W: %i]\n", readers_counter - reading, writers_counter - writing, reading, writing);
        usleep(rand() % 1000000); // Symulacja czasu czytania

        pthread_mutex_lock(&mutex); // Blokada dostępu do zmiennych współdzielonych
        reading--; // Zmniejszenie liczby czytających czytelników
        if (reading == 0) {
            pthread_mutex_unlock(&lock_writer); // Odblokowanie dla pisarzy, gdy ostatni czytelnik wychodzi
        }
        pthread_mutex_unlock(&mutex); // Odblokowanie dostępu do zmiennych współdzielonych

        usleep(rand() % 1000000); // Symulacja czekania w kolejce
    }
    return NULL;
}

// Funkcja reprezentująca wątek pisarza
void *writer(void *w) {
    while (1) {
        pthread_mutex_lock(&lock_writer); // Blokada dla pisarzy. Pisarz wchodzi do czytelni
        writing++; // Zwiększenie liczby piszących pisarzy
        // Wypisanie stanu kolejki i czytelni
        printf("ReaderQ: %i WriterQ: %i [in: R: %i W: %i]\n", readers_counter - reading, writers_counter - writing, reading, writing);
        usleep(rand() % 1000000); // Symulacja czasu pisania
        writing--; // Zmniejszenie liczby piszących pisarzy
        pthread_mutex_unlock(&lock_writer); // Odblokowanie dla pisarzy

        usleep(rand() % 1000000); // Symulacja czasu pomiędzy sesjami pisania
    }
    return NULL;
}

// Funkcja główna programu
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Podaj program Liczba_Czytelników Liczba_Pisarzy\n");
        return 1; // Błąd: nieprawidłowa liczba argumentów
    }

    R = stringToInt(argv[1]); // Pobranie liczby czytelników z argumentu
    W = stringToInt(argv[2]); // Pobranie liczby pisarzy z argumentu

    pthread_mutex_init(&mutex, NULL); // Inicjalizacja muteksu
    pthread_mutex_init(&lock_writer, NULL); // Inicjalizacja muteksu dla pisarzy

    pthread_t readers[R]; // Tablica wątków czytelników
    pthread_t writers[W]; // Tablica wątków pisarzy

    readers_counter = R; // Inicjalizacja liczby czytelników czekających na czytanie
    writers_counter = W; // Inicjalizacja liczby pisarzy czekających na pisanie

    // Tworzenie wątków czytelników
    for (int i = 0; i < R; i++) {
        pthread_create(&readers[i], NULL, reader, NULL);
    }
    // Tworzenie wątków pisarzy
    for (int i = 0; i < W; i++) {
        pthread_create(&writers[i], NULL, writer, NULL);
    }
    // Oczekiwanie na zakończenie wątków czytelników
    for (int i = 0; i < R; i++) {
        pthread_join(readers[i], NULL);
    }
    // Oczekiwanie na zakończenie wątków pisarzy
    for (int i = 0; i < W; i++) {
        pthread_join(writers[i], NULL);
    }

    // Zniszczenie muteksów
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&lock_writer);

    return 0;
}