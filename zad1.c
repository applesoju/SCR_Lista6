#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#define MAPPED_FILE "mapFile"

int main(){

    char fileName[50];          // Tablica znaków przechowująca nazwę wczytywanego pliku
    int fd, outFile;            // deskryptory plików
    struct stat fileStatus;     // struktura informacji o pliku
    int *memPtr;                // wskaźnik na mapowaną pamięć
    
    while(1){
        printf("Podaj nazwę pliku: ");
        scanf("%s", fileName);          // Wczytujemy nazwę pliku

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

            close(fd);
        }
    }

    return 0;
}
