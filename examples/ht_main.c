#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hash_file.h"
#include <unistd.h>

#define RECORDS_NUM 1000 // you can change it if you want
#define GLOBAL_DEPT 2    // you can change it if you want
#define FILE_NAME "data.db"

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
  /*
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


  CALL_OR_DIE(HT_CloseFile(indexDesc));
  BF_Close();
  */

  printf("start...\n\n");
  ////////////////////////////////////////////////////////////////////////////////////////////////
  
  Record record, record1, record2;

  record.id = 1;
  strcpy(record.city, cities[0]);
  strcpy(record.name, names[0]);
  strcpy(record.surname, surnames[0]);

  record1.id = 2;
  strcpy(record1.city, cities[1]);
  strcpy(record1.name, names[1]);
  strcpy(record1.surname, surnames[1]);

  record2.id = 3;
  strcpy(record2.city, cities[2]);
  strcpy(record2.name, names[2]);
  strcpy(record2.surname, surnames[2]);


  int id;
  int depth = 1;
  const char *filename = "temp.db";
  int want = 2;
  ////////////////////////////////////////////////////////////////////////////////////////////////

  HT_Init();
  HT_CreateIndex(filename, depth);
  HT_OpenIndex(filename, &id);
  
  printf("\n");
  HT_InsertEntry(id, record);
  HT_InsertEntry(id, record1);
  HT_InsertEntry(id, record2);
  
  printf("\n");
  // HT_PrintAllEntries(id, &want);
  HT_PrintAllEntries(id, NULL);
  
  HT_CloseFile(id);

  ////////////////////////////////////////////////////////////////////////////////////////////////
  char *test_name = "temp.db";
  //HashStatistics(test_name); //it shouldn't work on closed file
  printf("\n...end\n");
  return 0;
}
