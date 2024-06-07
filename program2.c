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



// Program symulujący zagłodzenie czytelników przed czytelnią

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Deklaracja zmiennych globalnych
int R; // Liczba czytelników
int W; // Liczba pisarzy

int readers_counter = 0; // Licznik czytelników
int writers_counter = 0; // Licznik pisarzy

int reading = 0; // Liczba aktualnie czytających czytelników
int writing = 0; // Liczba aktualnie piszących pisarzy

int waiting_writers = 0; // Liczba pisarzy czekających na pisanie
int waiting_readers = 0; // Liczba czytelników czekających na czytanie

// Deklaracja muteksów
pthread_mutex_t reading_variable_mutex;
pthread_mutex_t waiting_writers_variable_mutex;
pthread_mutex_t lock_writer;
pthread_mutex_t lock_reader;

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

// Funkcja generująca losowy czas oczekiwania
void wait() {
    int wait_time = rand() % 1000000 + 1; // Generowanie losowego czasu oczekiwania
    usleep(wait_time); // Oczekiwanie przez losowy czas
}

// Funkcja reprezentująca wątek czytelnika
void* reader(void *r) {
    srand(time(NULL)); // Inicjalizacja generatora liczb losowych
    while (1) {
        pthread_mutex_lock(&lock_reader); // Blokada dla czytelników
        pthread_mutex_lock(&reading_variable_mutex); // Blokada zmiennej reading
        if (reading == 0) {
            pthread_mutex_lock(&lock_writer); // Blokada dla pisarzy, jeśli żaden czytelnik nie czyta
        }
        reading++; // Zwiększenie liczby czytelników
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", readers_counter - reading, writers_counter - writing, reading, writing);
        fflush(stdout); // Czyszczenie bufora
        pthread_mutex_unlock(&reading_variable_mutex); // Odblokowanie zmiennej reading
        pthread_mutex_unlock(&lock_reader); // Odblokowanie dla czytelników

        wait(); // Symulacja czytania

        pthread_mutex_lock(&reading_variable_mutex); // Blokada zmiennej reading
        reading--; // Zmniejszenie liczby czytelników
        if (reading == 0) {
            pthread_mutex_unlock(&lock_writer); // Odblokowanie dla pisarzy, jeśli żaden czytelnik nie czyta
        }
        pthread_mutex_unlock(&reading_variable_mutex); // Odblokowanie zmiennej reading

        wait(); // Symulacja czasu pomiędzy sesjami czytania
    }
    return NULL;
}

// Funkcja reprezentująca wątek pisarza
void* writer(void *w) {
    srand(time(NULL)); // Inicjalizacja generatora liczb losowych
    while (1) {
        pthread_mutex_lock(&waiting_writers_variable_mutex); // Blokada zmiennej waiting_writing
        waiting_writers++; // Zwiększenie liczby czekających pisarzy
        if (waiting_writers == 1) {
            pthread_mutex_lock(&lock_reader); // Blokada dla czytelników, jeśli jest to pierwszy czekający pisarz
        }
        pthread_mutex_unlock(&waiting_writers_variable_mutex); // Odblokowanie zmiennej waiting_writing

        pthread_mutex_lock(&lock_writer); // Blokada dla pisarzy
        writing++; // Zwiększenie liczby pisarzy
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", readers_counter - reading, writers_counter - writing, reading, writing);
        fflush(stdout); // Czyszczenie bufora
        wait(); // Symulacja pisania
        writing--; // Zmniejszenie liczby pisarzy
        pthread_mutex_unlock(&lock_writer); // Odblokowanie dla pisarzy

        pthread_mutex_lock(&waiting_writers_variable_mutex); // Blokada zmiennej waiting_writing
        waiting_writers--; // Zmniejszenie liczby czekających pisarzy
        if (waiting_writers == 0) {
            pthread_mutex_unlock(&lock_reader); // Odblokowanie dla czytelników, jeśli nie ma więcej czekających pisarzy
        }
        pthread_mutex_unlock(&waiting_writers_variable_mutex); // Odblokowanie zmiennej waiting_writing

        wait(); // Symulacja czasu pomiędzy sesjami pisania
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
    
     if(R < 1 || W < 1) {
        printf("Podaj poprawne liczby\n");
        return 1; // Błąd: nieprawidłowe liczby
    }
    // Inicjalizacja muteksów
    pthread_mutex_init(&reading_variable_mutex, NULL);
    pthread_mutex_init(&waiting_writers_variable_mutex, NULL);
    pthread_mutex_init(&lock_writer, NULL);
    pthread_mutex_init(&lock_reader, NULL);

    pthread_t readers[R]; // Tablica wątków czytelników
    pthread_t writers[W]; // Tablica wątków pisarzy

    readers_counter = R; // Inicjalizacja licznika czytelników
    writers_counter = W; // Inicjalizacja licznika pisarzy

    // Tworzenie wątków czytelników
    for (int i = 0; i < R; i++) {
        pthread_create(&readers[i], NULL, reader, NULL);
    }
    // Tworzenie wątków pisarzy
    for (int i = 0; i < W; i++) {
        pthread_create(&writers[i], NULL, writer, NULL);
    }
    // Oczekiwanie na zakończenie wątków czytelników
    //for (int i = 0; i < R; i++) {
        pthread_join(readers[0], NULL);
    //}
    // Oczekiwanie na zakończenie wątków pisarzy
    //for (int i = 0; i < W; i++) {
    //   pthread_join(writers[i], NULL);
    //}

    return 0;
}
