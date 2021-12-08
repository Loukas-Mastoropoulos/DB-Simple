#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HT_ERROR;        \
  }                         \
}

HT_ErrorCode HT_Init() {
  //insert code here
  //printf("This is HT_Init start\n");
  CALL_BF(BF_Init(LRU));
  //printf("End of HT_Init\n");
  return HT_OK;
}

HT_ErrorCode HT_CreateIndex(const char *filename, int depth) {
  //insert code here
  printf("Name given : %s, max depth : %i\n", filename, depth);
  CALL_BF(BF_CreateFile(filename));
  printf("file was not created before\n");
  
  return HT_OK;
}

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
  //insert code here
  printf("Entering HT_OpenIndex\n");
  CALL_BF(BF_OpenFile(fileName, indexDesc));
  printf("Exiting HT_OpenIndex\n");

  return HT_OK;
}

HT_ErrorCode HT_CloseFile(int indexDesc) {
  //insert code here
  printf("Entering HT_CloseFile\n");
  CALL_BF(BF_CloseFile(indexDesc));
  printf("Exiting HT_CloseFile\n");

  return HT_OK;
}

void printRecord(Record record){
  printf("Entry with id : %i, city : %s, name : %s, and surname : %s\n", 
          record.id, record.city, record.name, record.surname);
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  //insert code here
  printf("Entering HT_InsertEntry\n");
  
  printRecord(record);

  BF_Block *block;
  BF_Block_Init(&block);

  CALL_BF(BF_AllocateBlock(indexDesc, block));
  char *data = BF_Block_GetData(block);
  memcpy(data, &record, sizeof(Record));
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);

  printf("Exiting HT_InsertEntry\n");

  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  printf("Entering HT_PrintAllEntries\n");
  BF_Block *block;
  BF_Block_Init(&block);
  
  int i = 2;
  Record record;

  CALL_BF(BF_GetBlock(indexDesc, i, block));
  char *data = BF_Block_GetData(block);
  memcpy(&record, data, sizeof(Record));
  printRecord(record);
  CALL_BF(BF_UnpinBlock(block));
  
  BF_Block_Destroy(&block);
  printf("Exiting HT_PrintAllEntries\n");

  return HT_OK;
}
