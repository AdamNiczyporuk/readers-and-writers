// Czytelnicy i pisarze. Z czytelni korzysta na okrągło pewna ilość czytelników i pisarzy, przy czym jednocześnie może w niej znajdować się albo dowolna ilość czytelników,
// albo jeden pisarz, albo nikt - nigdy inaczej. Problem ten ma trzy rozwiązania - z możliwością zagłodzenia pisarzy,
// z możliwością zagłodzenia czytelników oraz wykluczające zagłodzenie. Napisać:

// a) dwa programy symulujące dwa różne rozwiązania tego problemu, bez korzystania ze zmiennych warunkowych [17 p], lub
// b) dwa programy symulujące dwa różne rozwiązania tego problemu, przy czym jeden z nich musi korzystać ze zmiennych warunkowych (condition variable). [27 p], lub
// c) trzy programy symulujące trzy różne rozwiązania tego problemu, przy czym przynajmniej jeden z nich musi korzystać ze zmiennych warunkowych [34 p].

// Ilość wątków pisarzy R i czytelników W można przekazać jako argumenty linii poleceń. Zarówno czytelnicy jak i pisarze wkrótce po opuszczeniu czytelni próbują znów się do niej dostać.
// Program powinien wypisywać komunikaty według poniższego przykładu:

// ReaderQ: 11 WriterQ: 10 [in: R:0 W:1]
// Oznacza to, że w kolejce przed czytelnią czeka 10 pisarzy i 11 czytelników, a sama czytelnia zajęta jest przez jednego pisarza. Komunikat należy wypisywać w momencie zmiany którejkolwiek z tych zmiennych.

#include <unistd.h>  // Biblioteka do funkcji usleep
#include <stdio.h>   // Biblioteka do funkcji printf
#include <pthread.h> // Biblioteka do tworzenia i zarządzania wątkami
#include <stdlib.h>  // Biblioteka do funkcji związanych z alokacją pamięci, konwersją i generowaniem liczb losowych

// Deklaracja zmiennych globalnych
int R; // Liczba czytelników
int W; // Liczba pisarzy

int reading = 0;  // Liczba aktualnie czytających
int writting = 0; // Liczba aktualnie piszących

int waiting_writers = 0; // Liczba oczekujących pisarzy
int waiting_readers = 0; // Liczba oczekujących czytelników

pthread_mutex_t mutex;      // Muteks do synchronizacji dostępu do sekcji krytycznej
pthread_cond_t cond_reader; // Zmienna warunkowa dla czytelników
pthread_cond_t cond_writer; // Zmienna warunkowa dla pisarzy

// Funkcja generująca losowy czas oczekiwania (symulująca czas czytania/pisania)
void wait()
{
    int wait_time = rand() % 1000000 + 1; // Losowy czas oczekiwania od 1 do 1000000 mikrosekund
    usleep(wait_time);                    // Funkcja usypiająca wątek na określony czas
}

// Funkcja konwertująca łańcuch znaków na liczbę całkowitą
int stringToInt(const char *str)
{
    if (str == NULL || *str == '\0')
    { // Sprawdzenie, czy łańcuch znaków jest pusty
        return -1;
    }

    char *endptr;
    long int value = strtol(str, &endptr, 10); // Konwersja łańcucha znaków na liczbę całkowitą

    if (endptr == str || *endptr != '\0')
    { // Sprawdzenie, czy nie nastąpił błąd podczas konwersji
        return -1;
    }

    return (int)value; // Zwrócenie wyniku konwersji
}


// Funkcja dla wątku czytelnika
void *reader(void *r)
{

    srand(time(NULL)); // Inicjalizacja generatora liczb losowych
    while (1)
    {
        // początek czytania
        pthread_mutex_lock(&mutex); // Zablokowanie muteksu

        if (waiting_writers > 0 || writting > 0)
        {                                            // Jeśli są oczekujący pisarze lub ktoś pisze
            waiting_readers++;  
            printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", waiting_readers, waiting_writers ,reading, writting);                     // Zwiększenie liczby oczekujących czytelników
            pthread_cond_wait(&cond_reader, &mutex); // Oczekiwanie na zmienną warunkową czytelnika
        }
        else
        {
            reading++; // Zwiększenie liczby aktualnie czytających
            printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", waiting_readers, waiting_writers ,reading, writting);
        }

        pthread_mutex_unlock(&mutex); // Odblokowanie muteksu
        wait();           // Symulacja czasu czytania

        // koniec czytania
        pthread_mutex_lock(&mutex); // Zablokowanie muteksu
        reading--;                  // Zmniejszenie liczby aktualnie czytających
        if (reading == 0)
        {
            pthread_cond_signal(&cond_writer); // Powiadomienie pisarzy, że można pisać
        }
        pthread_mutex_unlock(&mutex); // Odblokowanie muteksu
        wait();           // Symulacja czasu oczekiwania przed ponownym czytaniem
    }
    return NULL;
}

// Funkcja dla wątku pisarza
void *writer(void *w)
{
    srand(time(NULL)); // Inicjalizacja generatora liczb losowych
    while (1)
    {
        // początek pisania
        pthread_mutex_lock(&mutex); // Zablokowanie muteksu

        if (reading > 0 || writting > 0)
        {                                            // Jeśli ktoś czyta lub pisze
            waiting_writers++;                       // Zwiększenie liczby oczekujących pisarzy
            printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", waiting_readers, waiting_writers ,reading, writting);
            pthread_cond_wait(&cond_writer, &mutex); // Oczekiwanie na zmienną warunkową pisarza
            
            waiting_writers--;
        }

        writting = 1; // Pisarz wszedł do czytelni
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", waiting_readers, waiting_writers , reading, writting);

        pthread_mutex_unlock(&mutex); // Odblokowanie muteksu
        wait();           // Symulacja czasu pisania

        // koniec pisania
        pthread_mutex_lock(&mutex);   // Zablokowanie muteksu
        writting = 0;                 // Zresetowanie flagi pisania

        if (waiting_readers == 0)
        {
            pthread_cond_signal(&cond_writer); // Powiadomienie jednego pisarza
        }
        else
        {
            pthread_cond_broadcast(&cond_reader); // Powiadomienie wszystkich oczekujących czytelników
            reading = reading + waiting_readers;  // Aktualizacja liczby czytających
            waiting_readers = 0;                  // Resetowanie liczby oczekujących czytelników
            printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n", waiting_readers, waiting_writers , reading, writting);
        }

        pthread_mutex_unlock(&mutex); // Odblokowanie muteksu
        wait();           // Symulacja czasu oczekiwania przed ponownym pisaniem
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    { // Sprawdzenie, czy podano poprawną liczbę argumentów
        printf("Podaj program Liczba_Czytelników Liczba_Pisarzy\n");
        return 1;
    }

    // Konwersja argumentów na liczby całkowite
    R = stringToInt(argv[1]);
    W = stringToInt(argv[2]);

    if (R < 1 || W < 1)
    {
        printf("Podaj poprawne liczby\n");
        return 1; // Błąd: nieprawidłowe liczby
    }
    // Inicjalizacja muteksu i zmiennych warunkowych
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_reader, NULL);
    pthread_cond_init(&cond_writer, NULL);

    pthread_t readers[R]; // Tablica wątków czytelników
    pthread_t writers[W]; // Tablica wątków pisarzy

   

    // Tworzenie wątków czytelników
    for (int i = 0; i < R; i++)
    {
        pthread_create(&readers[i], NULL, reader, NULL);
    }

    // Tworzenie wątków pisarzy
    for (int i = 0; i < W; i++)
    {
        pthread_create(&writers[i], NULL, writer, NULL);
    }

    // Dołączanie wątków czytelników
    //for (int i = 0; i < R; i++)
    //{
        pthread_join(readers[0], NULL); //Wystarczy oczekiwać na jeden wątek, żeby program się nie zakończył
    //}

    // Dołączanie wątków pisarzy
    //for (int i = 0; i < W; i++)
    //{
    //    pthread_join(writers[i], NULL);
    //}

    return 0;
}
