#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hash_file.h"
#include <unistd.h>

#define RECORDS_NUM 277 // you can change it if you want
#define GLOBAL_DEPT 2    // you can change it if you want
#define FILE_NAME "data.db"

// Assumes little endian
void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    
    for (i = size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

const char *names[] = {
    "Yannis",
    "Christofos",
    "Sofia",
    "Marianna",
    "Vagelis",
    "Maria",
    "Iosif",
    "Dionisis",
    "Konstantina",
    "Theofilos",
    "Giorgos",
    "Dimitris"};

const char *surnames[] = {
    "Ioannidis",
    "Svingos",
    "Karvounari",
    "Rezkalla",
    "Nikolopoulos",
    "Berreta",
    "Koronis",
    "Gaitanis",
    "Oikonomou",
    "Mailis",
    "Michas",
    "Halatsis"};

const char *cities[] = {
    "Athens",
    "San Francisco",
    "Los Angeles",
    "Amsterdam",
    "London",
    "New York",
    "Tokyo",
    "Hong Kong",
    "Munich",
    "Miami"};

#define CALL_OR_DIE(call)     \
  {                           \
    HT_ErrorCode code = call; \
    if (code != HT_OK)        \
    {                         \
      printf("Error\n");      \
      exit(code);             \
    }                         \
  }

int main()
{
  
  BF_Init(LRU);

  CALL_OR_DIE(HT_Init());

  int indexDesc;
  CALL_OR_DIE(HT_CreateIndex(FILE_NAME, GLOBAL_DEPT));
  CALL_OR_DIE(HT_OpenIndex(FILE_NAME, &indexDesc));

  Record record;
  srand(12569874);
  int r;
  printf("Insert Entries\n");
  for (int id = 0; id < RECORDS_NUM; ++id) {
    // create a record
    record.id = id;
    r = rand() % 12;
    memcpy(record.name, names[r], strlen(names[r]) + 1);
    r = rand() % 12;
    memcpy(record.surname, surnames[r], strlen(surnames[r]) + 1);
    r = rand() % 10;
    memcpy(record.city, cities[r], strlen(cities[r]) + 1);

    CALL_OR_DIE(HT_InsertEntry(indexDesc, record));
  }

  printf("RUN PrintAllEntries\n");
  int id = rand() % RECORDS_NUM;
  CALL_OR_DIE(HT_PrintAllEntries(indexDesc, &id));
  //CALL_OR_DIE(HT_PrintAllEntries(indexDesc, NULL));

  printf("RUN HashStatistics\n");
  CALL_OR_DIE(HashStatistics(FILE_NAME));
  CALL_OR_DIE(HT_CloseFile(indexDesc));
  BF_Close();
  

  // printf("start...\n\n");
  // ////////////////////////////////////////////////////////////////////////////////////////////////

  // srand(time(NULL));

  // Record record;
  // int id, r;
  // int depth = 1;
  // const char *filename = "temp.db";
  // int want = 13;
  // int num = 17;
  // ////////////////////////////////////////////////////////////////////////////////////////////////

  // CALL_OR_DIE(HT_Init());
  // CALL_OR_DIE(HT_CreateIndex(filename, depth));  //Αν βαλουμε CALL_OR_DIE εδω δεν μπορουμε να το τρεξουμε 2η φορα,που απο ο,τι καταλαβαινω ετσι θα επρεπε
  // CALL_OR_DIE(HT_OpenIndex(filename, &id));
  
  // printf("\n");
  // for (int j = 0; j < num; ++j) {
  //   // create a record
  //   record.id = j;
  //   r = rand() % 12;
  //   memcpy(record.name, names[r], strlen(names[r]) + 1);
  //   r = rand() % 12;
  //   memcpy(record.surname, surnames[r], strlen(surnames[r]) + 1);
  //   r = rand() % 10;
  //   memcpy(record.city, cities[r], strlen(cities[r]) + 1);

  //   HT_InsertEntry(id, record);
  // }
  
  // printf("\n");
  // CALL_OR_DIE(HT_PrintAllEntries(id, &want));

  // //printf("\n");
  // //HT_PrintAllEntries(id, NULL);
  
  // CALL_OR_DIE(HT_CloseFile(id));

  // // ////////////////////////////////////////////////////////////////////////////////////////////////
  // char *test_name = "temp.db";
  // HashStatistics(test_name); //it shouldn't work on closed file
  // // printf("\n...end\n");
  return 0;
}
