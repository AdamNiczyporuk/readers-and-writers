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

///ZAGŁODZENIE CZYTELNIKÓW !!!!!!!!!!!!!!!!!!!!!!
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
int R; 
int W;

int readers_counter =0;
int writers_counter=0;

int reading=0;
int writting =0 ; 

int waiting_writing=0;
pthread_mutex_t reader_mutex;
pthread_mutex_t writer_mutex;
pthread_mutex_t lock_writer;
pthread_mutex_t lock_reader;

int stringToInt(const char *str) {
    // Sprawdź, czy łańcuch znaków jest pusty
    if (str == NULL || *str == '\0') {
        return -1;
    }

    // Użyj funkcji strtol do konwersji łańcucha znaków na liczbę całkowitą
    char *endptr;
    long int value = strtol(str, &endptr, 10);

    // Sprawdź, czy nie nastąpił błąd podczas konwersji
    if (endptr == str || *endptr != '\0') {
        return -1;
    }
    
    // Zwróć wynik konwersji
    return (int)value;
}
void random_wait_time() 
{
        
        int wait_time = rand() % 1000000 + 1; // Generowanie losowego czasu oczekiwania
        usleep(wait_time); 
}

void* reader(void *r)
{ 
    srand(time(NULL));  
    while(1)
    {
        pthread_mutex_lock(&lock_reader);
        pthread_mutex_lock(&reader_mutex);
        if(reading == 0)
        {
            pthread_mutex_lock(&lock_writer);
        }
        reading ++ ;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n",readers_counter-reading,writers_counter-writting,reading,writting);

        pthread_mutex_unlock(&reader_mutex);
        pthread_mutex_unlock(&lock_reader);

        random_wait_time();

        pthread_mutex_lock(&reader_mutex);
        reading--;
        if(reading == 0)
        {
            pthread_mutex_unlock(&lock_writer);
        }
        pthread_mutex_unlock(&reader_mutex);

        random_wait_time() ;
    }
    return NULL;
}

void* writer(void *w)
{
    srand(time(NULL));
    while(1)
    {
        pthread_mutex_lock(&writer_mutex); 
        waiting_writing++;
        if(waiting_writing==1)
        {
            pthread_mutex_lock(&lock_reader);
        }
        pthread_mutex_unlock(&writer_mutex);


        pthread_mutex_lock(&lock_writer);
        writting++;
        printf("ReaderQ: %d WriterQ: %d [in: R:%d W:%d]\n",readers_counter-reading,writers_counter-writting,reading,writting);
        random_wait_time();
        writting--;
        pthread_mutex_unlock(&lock_writer);


        pthread_mutex_lock(&writer_mutex);
        waiting_writing--;
        if(waiting_writing==0)
        {
            pthread_mutex_unlock(&lock_reader);
        }
        pthread_mutex_unlock(&writer_mutex);

        random_wait_time();
    }
    return NULL;
}






int main(int argc, char  *argv[])
{
    if (argc !=3)
    { 
        printf("Podaj program Liczba_Czytelników Liczba_Pisarzy\n");
        return 1;
    }
    R = stringToInt(argv[1]);
    W = stringToInt(argv[2]);

    pthread_mutex_init(&reader_mutex,NULL);
    pthread_mutex_init(&writer_mutex,NULL);
    pthread_mutex_init(&lock_writer,NULL);
    pthread_mutex_init(&lock_reader,NULL);

 

    pthread_t readers[R];
    pthread_t writers[W];

   readers_counter = R;
    writers_counter = W;

    

    for(int i =0 ;i<R;i++)
    {
        pthread_create(&readers[i],NULL,reader,NULL);
    }
    for(int i=0; i<W;i++)
    {
        pthread_create(&writers[i],NULL,writer,NULL);
    }
    for(int i=0; i<R;i++)
    {
        pthread_join(readers[i],NULL); 
    }
    for(int i =0; i<W ;i++)
    {
        pthread_join(writers[i],NULL);
    }

    return 0;
}
