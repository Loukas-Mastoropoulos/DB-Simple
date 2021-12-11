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

typedef struct{
  Record record[3];
} Entry;

typedef struct{
  int fd;
  int used;
} IndexNode;

int dataN;
static int size;

IndexNode indexArray[MAX_OPEN_FILES];
HT_ErrorCode HT_Init() {

  //printf("This is HT_Init start\n");
  CALL_BF(BF_Init(LRU));
  for(int i = 0; i < MAX_OPEN_FILES; i++)indexArray[i].used = 0;
  size = 0;
  
  //printf("End of HT_Init\n");
  return HT_OK;

}

HT_ErrorCode HT_CreateIndex(const char *filename, int depth) {
  
  printf("Name given : %s, max depth : %i\n", filename, depth);
  CALL_BF(BF_CreateFile(filename));

  BF_Block *block;
  BF_Block_Init(&block);

  int id;
  HT_OpenIndex(filename, &id);
  int fd = indexArray[id].fd;

  //create new block at end
  CALL_BF(BF_AllocateBlock(fd, block));
  char *data = BF_Block_GetData(block);
  memcpy(data, &depth, sizeof(int));
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);
  
  printf("file was not created before\n");

  HT_CloseFile(id);
  
  return HT_OK;

}

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
  
  //printf("Entering HT_OpenIndex\n");
  
  int found = 0;
  
  //find empty spot
  for(int i = 0; i < MAX_OPEN_FILES; i++){

    if(indexArray[i].used == 0){

      (*indexDesc) = i;
      found = 1;
      break;

    }

  }

  //if table is full return error
  if(found == 0)return HT_ERROR;

  int fd;
  CALL_BF(BF_OpenFile(fileName, &fd));
  int pos = (*indexDesc);
  indexArray[pos].fd = fd;
  
  //printf("Exiting HT_OpenIndex\n");

  return HT_OK;

}

HT_ErrorCode HT_CloseFile(int indexDesc) {
  
  //printf("Entering HT_CloseFile\n");
  int fd = indexArray[indexDesc].fd;
  CALL_BF(BF_CloseFile(fd));
  
  //printf("Exiting HT_CloseFile\n");

  return HT_OK;

}

void printRecord(Record record){
  printf("Entry with id : %i, city : %s, name : %s, and surname : %s\n", 
          record.id, record.city, record.name, record.surname);
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  
  //printf("Entering HT_InsertEntry\n");
  Entry entry;
  printRecord(record);
  int fd = indexArray[indexDesc].fd;

  BF_Block *block;
  BF_Block_Init(&block);
  int blockN;

  if(size == 0){
    //create new block at end
    CALL_BF(BF_AllocateBlock(fd, block));
  }
  else{
    //get last block
    BF_GetBlockCounter(fd, &blockN);
    blockN = blockN - 1;
    CALL_BF(BF_GetBlock(fd, blockN, block));
  }

  char *data = BF_Block_GetData(block);
  if(size != 0)memcpy(&entry, data, sizeof(Entry)); //get previous data
  entry.record[size] = record;  //add record
 
  memcpy(data, &entry, sizeof(Entry));
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);

  size++; //increase size of file
  //printf("Exiting HT_InsertEntry\n");

  return HT_OK;

}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  
  printf("Entering HT_PrintAllEntries\n");
  BF_Block *block;
  BF_Block_Init(&block);
  
  int fd = indexArray[indexDesc].fd;
  int i = 1;
  Entry entry;

  CALL_BF(BF_GetBlock(fd, i, block));
  char *data = BF_Block_GetData(block);
  memcpy(&entry, data, sizeof(Entry));
  
  //print records with specific id
  for(int i = 0; i < size; i++)
    if(id == NULL)printRecord(entry.record[i]);   //if id not given, print all records
    else
      if(entry.record[i].id == (*id))printRecord(entry.record[i]);

  CALL_BF(BF_UnpinBlock(block));
  
  BF_Block_Destroy(&block);
  //printf("Exiting HT_PrintAllEntries\n");

  return HT_OK;

}
