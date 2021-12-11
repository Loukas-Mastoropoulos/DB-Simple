#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
  char desc[30];
  int size;
} DataHeader;

typedef struct{
  DataHeader header;
  Record record[3];
} Entry;

typedef struct{
  int fd;
  int used;
} IndexNode;

typedef struct{
  int value;
  int block_num;
} HashNode;

int dataN;

IndexNode indexArray[MAX_OPEN_FILES];
HT_ErrorCode HT_Init() {

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
  
  printf("Name given : %s, max depth : %i\n", filename, depth);
  CALL_BF(BF_CreateFile(filename));

  BF_Block *block;
  BF_Block_Init(&block);

  int id;
  HT_OpenIndex(filename, &id);
  int fd = indexArray[id].fd;
  HashNode hashNode[2];
  hashNode[0].value = 0;
  hashNode[0].block_num = 2;
  hashNode[1].value = 1;
  hashNode[1].block_num = 3;
  

  //create first block for info
  CALL_BF(BF_AllocateBlock(fd, block));
  char *data = BF_Block_GetData(block);
  memcpy(data, &depth, sizeof(int));
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));

  //create second block for hashing
  CALL_BF(BF_AllocateBlock(fd, block));
  data = BF_Block_GetData(block);
  memcpy(data, hashNode, 2*sizeof(hashNode));
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

  indexArray[pos].used = 1;  // Position pos in now taken (a.k.a being used)
  
  //printf("Exiting HT_OpenIndex\n");

  return HT_OK;

}

HT_ErrorCode HT_CloseFile(int indexDesc) {
  
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

void printHashNode(HashNode node){
  printf("HashNode with value : %i, and block_num : %i\n", node.value, node.block_num);
}

int hashFunction(int id){
  int depth = 2;
  int number_of_values = pow(2.0, (double)depth);
  return id % number_of_values;
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
  
  //printf("Entering HT_InsertEntry\n");
  Entry entry;
  printRecord(record);
  
  if(indexArray[indexDesc].used == 0){
    printf("Trying to access a closed file!\n");  // We can't/shouldn't insert into a closed file
    return HT_ERROR;    
  }
  
  int fd =indexArray[indexDesc].fd;

  BF_Block *block;
  BF_Block_Init(&block);
  int blockN;
  int new = 0;  //1 if we create new entry, 0 if we add to existing one
  BF_GetBlockCounter(fd, &blockN);

  if(blockN == 2){  //if there was no entry added to file before
    
    //create new block at end
    CALL_BF(BF_AllocateBlock(fd, block));
    strcpy(entry.header.desc, "test header");   //dummy header description
    entry.header.size = 0;
    new = 1;    //note that we have a new entry

  }
  else{

    //if we don't need to create new entry get last block
    blockN = blockN - 1;
    CALL_BF(BF_GetBlock(fd, blockN, block));
  }

  char *data = BF_Block_GetData(block);
  if(new == 0)memcpy(&entry, data, sizeof(Entry)); //get previous data, if we're adding to old entry
  entry.record[entry.header.size] = record;  //add record
  (entry.header.size) ++;
 
  memcpy(data, &entry, sizeof(Entry));
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);
  //printf("Exiting HT_InsertEntry\n");

  return HT_OK;

}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id) {
  
  printf("Entering HT_PrintAllEntries\n");
  BF_Block *block;
  BF_Block_Init(&block);
  
  int fd = indexArray[indexDesc].fd;
  int i = 2;    //get 2nd block, 1st has info about file
  Entry entry;
  // HashNode hashNode[2];

  //Testing 2nd block (block with hash codes)
  /*
  CALL_BF(BF_GetBlock(fd, 1, block));
  char *data = BF_Block_GetData(block);
  memcpy(hashNode, data, 2*sizeof(HashNode));

  printHashNode(hashNode[0]);
  printHashNode(hashNode[1]);
  */
  ///////////////////////////////////////////


  CALL_BF(BF_GetBlock(fd, i, block));
  char *data = BF_Block_GetData(block);
  memcpy(&entry, data, sizeof(Entry));
  
  //print header info
  printf("entry header is : %s\n", entry.header.desc);

  //print records with specific id
  for(int i = 0; i < entry.header.size; i++)
    if(id == NULL)printRecord(entry.record[i]);   //if id not given, print all records
    else
      if(entry.record[i].id == (*id))printRecord(entry.record[i]);

  CALL_BF(BF_UnpinBlock(block));
  
  BF_Block_Destroy(&block);
  //printf("Exiting HT_PrintAllEntries\n");

  return HT_OK;

}
