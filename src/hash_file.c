#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bf.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20

#define CALL_BF(call)         \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK)        \
    {                         \
      BF_PrintError(code);    \
      return HT_ERROR;        \
    }                         \
  }

typedef struct
{
  int size;
} DataHeader;

typedef struct 
{
  char desc[30];
} HashHeader;

typedef struct
{
  DataHeader header;
  Record record[(BF_BLOCK_SIZE - sizeof(DataHeader)) / sizeof(Record)];
} Entry;

typedef struct
{
  int value;
  int block_num;
} HashNode;

typedef struct
{
  HashHeader header;
  HashNode hashNode[2];
} HashEntry;

typedef struct
{
  int fd;
  int used;
} IndexNode;

int max_entries;
int max_hNodes;

IndexNode indexArray[MAX_OPEN_FILES];
HT_ErrorCode HT_Init()
{

  if (MAX_OPEN_FILES == 0)
  {
    printf("Runner.exe needs at least one file to run. Please ensure that MAX_OPEN_FILES is not 0\n");
    return HT_ERROR;
  }

  CALL_BF(BF_Init(LRU));
  for (int i = 0; i < MAX_OPEN_FILES; i++)
    indexArray[i].used = 0;

  int max_records = (BF_BLOCK_SIZE - sizeof(DataHeader)) / sizeof(Record);
  int max_hNodes = BF_BLOCK_SIZE / sizeof(HashNode);
  
  return HT_OK;
}

HT_ErrorCode HT_CreateIndex(const char *filename, int depth)
{

  printf("Name given : %s, max depth : %i\n", filename, depth);
  CALL_BF(BF_CreateFile(filename));

  BF_Block *block;
  BF_Block_Init(&block);

  int id;
  HT_OpenIndex(filename, &id);
  int fd = indexArray[id].fd;
  
  HashEntry hashEntry;
  strcpy(hashEntry.header.desc, "dummy description");
  hashEntry.hashNode[0].value = 0;
  hashEntry.hashNode[0].block_num = 0;
  hashEntry.hashNode[1].value = 1;
  hashEntry.hashNode[1].block_num = 0;

  // create first block for info
  CALL_BF(BF_AllocateBlock(fd, block));
  char *data = BF_Block_GetData(block);
  memcpy(data, &depth, sizeof(int));
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));

  // create second block for hashing
  CALL_BF(BF_AllocateBlock(fd, block));
  data = BF_Block_GetData(block);
  memcpy(data, &hashEntry, sizeof(HashEntry));
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));

  BF_Block_Destroy(&block);
  printf("file was not created before\n");

  HT_CloseFile(id);

  return HT_OK;
}

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc)
{
  
  int found = 0;  //bool flag.

  // find empty spot
  for (int i = 0; i < MAX_OPEN_FILES; i++)
    if (indexArray[i].used == 0)
    {
      (*indexDesc) = i;
      found = 1;
      break;
    }

  // if table is full return error
  if (found == 0)return HT_ERROR;

  int fd;
  CALL_BF(BF_OpenFile(fileName, &fd));
  int pos = (*indexDesc);   // Return position
  indexArray[pos].fd = fd;  // Save fileDesc
  indexArray[pos].used = 1; // Set position to used

  return HT_OK;
}

HT_ErrorCode HT_CloseFile(int indexDesc)
{

  if (indexArray[indexDesc].used == 0)
  {
    printf("Can't close an already closed file!\n");
    return HT_ERROR;
  }

  // printf("Entering HT_CloseFile\n");
  int fd = indexArray[indexDesc].fd;

  indexArray[indexDesc].used = 0; // Free up position
  indexArray[indexDesc].fd = -1;  // -1 means that there is no file in position indexDesc

  CALL_BF(BF_CloseFile(fd));

  return HT_OK;
}

void printRecord(Record record)
{
  printf("Entry with id : %i, city : %s, name : %s, and surname : %s\n",
         record.id, record.city, record.name, record.surname);
}

void printHashNode(HashNode node)
{
  printf("HashNode with value : %i, and block_num : %i\n", node.value, node.block_num);
}

int hashFunction(int id)
{
  int depth = 1;
  int number_of_values = pow(2.0, (double)depth);
  return id % number_of_values;
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record)
{

  Entry entry;
  printRecord(record);

  //File is closed
  if (indexArray[indexDesc].used == 0)
  {
    printf("Trying to access a closed file!\n");
    return HT_ERROR;
  }

  int fd = indexArray[indexDesc].fd;

  BF_Block *block;
  BF_Block_Init(&block);
  int blockN;
  int new = 0; //bool flag. 1 if we create new block, else 0

  BF_GetBlockCounter(fd, &blockN);
  int value = hashFunction(record.id);

  HashEntry hashEntry;

  //get hashNodes
  CALL_BF(BF_GetBlock(fd, 1, block));
  char *data = BF_Block_GetData(block);
  memcpy(&hashEntry, data, sizeof(HashEntry));

  //find bucket
  int pos;
  for(pos = 0; pos < 2; pos++)if(hashEntry.hashNode[pos].value == value)break;

  //Check for allocated space

  if(hashEntry.hashNode[pos].block_num == 0){
    //no space was allocated

    //update hashNode values
    hashEntry.hashNode[pos].block_num = blockN;
    memcpy(data, &hashEntry, sizeof(HashEntry));
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));

    //allocate block at the end
    CALL_BF(BF_AllocateBlock(fd, block));
    //strcpy(entry.header.desc, "test header"); // dummy header description
    entry.header.size = 0;
    
    new = 1;   

  }else{
    //space has been previously allocated
    
    blockN = hashEntry.hashNode[pos].block_num; //get pointer to data block
    CALL_BF(BF_UnpinBlock(block));

    CALL_BF(BF_GetBlock(fd, blockN, block));

  }

  //write new info
  data = BF_Block_GetData(block);
  if(new == 0)memcpy(&entry, data, sizeof(Entry));  //if space was previously allocated, get previous data
  entry.record[entry.header.size] = record;         //add record
  (entry.header.size) ++;                           //update header size
 
  memcpy(data, &entry, sizeof(Entry));
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);

  return HT_OK;
}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *id)
{

  printf("Entering HT_PrintAllEntries\n");
  BF_Block *block;
  BF_Block_Init(&block);

  int fd = indexArray[indexDesc].fd;
  int blockN;
  
  Entry entry;
  HashEntry hashEntry;  

  //get hashNodes
  CALL_BF(BF_GetBlock(fd, 1, block));
  char *data = BF_Block_GetData(block);
  memcpy(&hashEntry, data, sizeof(HashEntry));
  CALL_BF(BF_UnpinBlock(block));

  //if id == NULL -> print all entries
  if(id == NULL){

    //for every hash value
    for(int i = 0; i < 2; i++){

      //get data block
      blockN = hashEntry.hashNode[i].block_num;
      CALL_BF(BF_GetBlock(fd, blockN, block));
      data = BF_Block_GetData(block);
      memcpy(&entry, data, sizeof(Entry));

      //print all records
      for (int i = 0; i < entry.header.size; i++)
        printRecord(entry.record[i]);

      CALL_BF(BF_UnpinBlock(block));


    }
    
    BF_Block_Destroy(&block);
    return HT_OK;

  }

  //id != NULL. Print specific entry
  int value = hashFunction((*id));

  //find data block_num
  int pos;
  for(pos = 0; pos < 2; pos++)if(hashEntry.hashNode[pos].value == value)break;
  blockN = hashEntry.hashNode[pos].block_num;

  if(blockN == 0){
    printf("Data block was not allocated");
    return HT_ERROR;
  }

  CALL_BF(BF_GetBlock(fd, blockN, block));
  data = BF_Block_GetData(block);
  memcpy(&entry, data, sizeof(Entry));

  //print record with that id
  for (int i = 0; i < entry.header.size; i++)
    if (entry.record[i].id == (*id))
      printRecord(entry.record[i]);

  CALL_BF(BF_UnpinBlock(block));

  BF_Block_Destroy(&block);

  return HT_OK;
}

HT_ErrorCode HashStatistics(char *filename)
{
  int id;
  HT_OpenIndex(filename, &id);
  int fd = indexArray[id].fd;

  // get number of blocks
  int nblocks;
  CALL_BF(BF_GetBlockCounter(fd, &nblocks));
  printf("File %s has %d blocks.\n", filename, nblocks);

  // [pending hashing and bucket creation]

  HT_CloseFile(id);

  return HT_OK;
}