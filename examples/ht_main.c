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
  
  Record record, record1, record2, r3, r4, r5, r6, r7;

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

  r3.id = 4;
  strcpy(r3.city, cities[3]);
  strcpy(r3.name, names[3]);
  strcpy(r3.surname, surnames[3]);

  r4.id = 5;
  strcpy(r4.city, cities[4]);
  strcpy(r4.name, names[4]);
  strcpy(r4.surname, surnames[4]);

  r5.id = 6;
  strcpy(r5.city, cities[5]);
  strcpy(r5.name, names[5]);
  strcpy(r5.surname, surnames[5]);

  r6.id = 7;
  strcpy(r6.city, cities[6]);
  strcpy(r6.name, names[6]);
  strcpy(r6.surname, surnames[6]);

  r7.id = 8;
  strcpy(r7.city, cities[7]);
  strcpy(r7.name, names[7]);
  strcpy(r7.surname, surnames[7]);
  

  int id;
  int depth = 1;
  const char *filename = "temp.db";
  int want = 3;
  ////////////////////////////////////////////////////////////////////////////////////////////////

  

  
  HT_Init();
  HT_CreateIndex(filename, depth);
  HT_OpenIndex(filename, &id);
  
  printf("\n");
  HT_InsertEntry(id, record);
  HT_InsertEntry(id, record1);
  HT_InsertEntry(id, record2);
  HT_InsertEntry(id, r3);
  HT_InsertEntry(id, r4);
  HT_InsertEntry(id, r5);
  HT_InsertEntry(id, r6);
  HT_InsertEntry(id, r7);
  
  
  
  printf("\n");
  HT_PrintAllEntries(id, &want);

  printf("\n");
  HT_PrintAllEntries(id, NULL);
  
  HT_CloseFile(id);

  ////////////////////////////////////////////////////////////////////////////////////////////////
  char *test_name = "temp.db";
  //HashStatistics(test_name); //it shouldn't work on closed file
  printf("\n...end\n");
  return 0;
}
