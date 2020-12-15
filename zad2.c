#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#define MAPPED_FILE "mapFile"       // nazwa pliku, do którego będziemy mapować pamięć
#define FIRST_IMG   "first.jpeg"    // nazwa obrazu, który będzie wyswietlany jako pierwszy
#define ARR_SIZE    50

int main(){

    char fileName[ARR_SIZE];    // Tablica znaków przechowująca nazwę wczytywanego pliku
    int fd, outFile;            // deskryptory plików
    struct stat fileStatus;     // struktura informacji o pliku
    int *memPtr;                // wskaźnik na mapowaną pamięć
    int empty = 0;

    pid_t PID = fork();         // tworzymy proces potomny

    if(PID == 0){               // kod dla dziecka
        sleep(1);               // opóźnienie
        execlp("display", "", "-update", "1", "-delay", "1", "mapFile", (char *)NULL);  // uruchomienie progrzamu "display"
    }

    if(PID > 0){                // kod dla rodzica

        printf("Podaj nazwę pliku lub wpisz 'e', aby zakończyć działanie programu:\n");

        sscanf(FIRST_IMG, "%s", fileName);                      // wpisujemy nazwę pierwszego obrazu do bufora nazw plików
        fd = open(fileName, O_RDONLY);                          // otwieramy plik w trybie odczytu
        outFile = open(MAPPED_FILE, O_RDWR | O_CREAT , 0666);   // otwieramy plik używany do mapowania lub go tworzymy jeśli nie istnieje
        fstat(fd, &fileStatus);                                 // zapisujemy informacje o pliku do struktury
        truncate(MAPPED_FILE, fileStatus.st_size);              // zmieniamy rozmiar pliku używanego do mapowania
        memPtr = mmap(NULL, fileStatus.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, outFile, 0);    // przygotowujemy pamięć
        read(fd, memPtr, fileStatus.st_size);                   // wczytujemy dane z obrazu do pliku używanego do mapowania

        while(1){
            scanf("%s", fileName);                              // Wczytujemy nazwę pliku

            if(fileName[0] == 'e' && strlen(fileName) == 1){    // Sprawdzamy czy wpisano 'e'
                printf("Koniec działania programu.\n");
                
                char command[20];
                snprintf(command, sizeof(command), "pkill -9 -P %d", PID);
                system(command);                                // zabijamy proces potomny
                return(0);                                      // i kończymy program
            }

            if((fd = open(fileName, O_RDONLY)) < 0) printf("Nie można otworzyć pliku.\n");
                // Otwieramy plik w trybie odczytu, jeśli się nie uda wyświetlamy komunikat
            else{   // W przeciwnym wypadku otwieramy plik używany do mapowania, a jeśli go nie ma to go tworzymy
                outFile = open(MAPPED_FILE, O_RDWR | O_CREAT , 0666);

                // próbujemy zapisać informacje dotyczące statusu pliku do bufora 'fileStatus'
                if(fstat(fd, &fileStatus) < 0){     // Jeśli się nie uda wyświetlamy komunikat i zwracamy -1
                    printf("Czytanie statusu pliku nie powiodło się.\n");
                    return(-1);
                }
                // Zmieniamy rozmiar pliku do mapowania na taki sam, co wczytany plik
                truncate(MAPPED_FILE, fileStatus.st_size);
                // Przygotowujemy pamięć z możliwością odczytu i zapisu w pliku 'outFile'
                // Użyto 'MAP_SHARED', aby można było sprawdzić zawartośc pliku innym procesem
                memPtr = mmap(NULL, fileStatus.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, outFile, 0); 
                // wczytujemy dane z wczytanego pliku i mapujemy je do pliku 'outFile'
                read(fd, memPtr, fileStatus.st_size);

                msync(memPtr, fileStatus.st_size, MS_SYNC);     // synchornizujemy plik ze zmapowaną pamięcią
                munmap(memPtr, fileStatus.st_size);             // odmapowujemy pamięć

                close(fd);                                      // zamykamy deskryptor
            }   
        }
    }

    return 0;
}