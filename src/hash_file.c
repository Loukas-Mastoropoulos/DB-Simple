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
  int fd;
  int used;
} indexNode;

indexNode indexArray[MAX_OPEN_FILES];
HT_ErrorCode HT_Init() {
  //insert code here
  //printf("This is HT_Init start\n");

  if(MAX_OPEN_FILES == 0){
    printf("Runner.exe needs at least one file to run.Please ensure that MAX_OPEN_FILES is not 0\n");
    return HT_ERROR;
  }

  CALL_BF(BF_Init(LRU));
  for(int i = 0; i < MAX_OPEN_FILES; i++)indexArray[i].used = 0;
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

  indexArray[pos].used = 1;  // Position pos in now taken (a.k.a being used)
  
  //printf("Exiting HT_OpenIndex\n");

  return HT_OK;
}

HT_ErrorCode HT_CloseFile(int indexDesc) {
  //insert code here
  //printf("Entering HT_CloseFile\n");
  
  int fd = indexArray[indexDesc].fd;
  
  indexArray[indexDesc].used=0;  //We have to "delete" the file from indexArray.We will just assume that this position can be reused
  indexArray[indexDesc].fd = -1; // -1 means that there is no file in position indexDesc
  
  CALL_BF(BF_CloseFile(fd));
  
  //printf("Exiting HT_CloseFile\n");

  return HT_OK;
}

void printRecord(Record record){
  printf("Entry with id : %i, city : %s, name : %s, and surname : %s\n", 
          record.id, record.city, record.name, record.surname);
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  //insert code here
  //printf("Entering HT_InsertEntry\n");
  
  printRecord(record);
  
  if(indexArray[indexDesc].used == 0){
    printf("Trying to access a closed file!\n");  // We can't/shouldn't insert into a closed file
    return HT_ERROR;    
  }
  
  int fd =indexArray[indexDesc].fd;


  BF_Block *block;
  BF_Block_Init(&block);

  CALL_BF(BF_AllocateBlock(fd, block));
  char *data = BF_Block_GetData(block);
  memcpy(data, &record, sizeof(Record));
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);

  //printf("Exiting HT_InsertEntry\n");

  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  //insert code here
  //printf("Entering HT_PrintAllEntries\n");
  
  if(indexArray[indexDesc].used == 0){
    printf("Can't print from a closed file! Please open the file first\n");
    return HT_ERROR;
  }

  BF_Block *block;
  BF_Block_Init(&block);
  
  int fd = indexArray[indexDesc].fd;
  int i = 0;
  Record record;

  CALL_BF(BF_GetBlock(fd, i, block));
  char *data = BF_Block_GetData(block);
  memcpy(&record, data, sizeof(Record));
  printRecord(record);
  CALL_BF(BF_UnpinBlock(block));
  
  BF_Block_Destroy(&block);
  //printf("Exiting HT_PrintAllEntries\n");

  return HT_OK;
}
