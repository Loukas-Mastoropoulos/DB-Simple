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
  int local_depth;
} DataHeader;

typedef struct 
{
  int size;
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
  HashNode hashNode[(BF_BLOCK_SIZE - sizeof(HashHeader)) / sizeof(HashNode)];
} HashEntry;

typedef struct
{
  int fd;
  int used;
} IndexNode;

int max_records;
int max_hNodes;

IndexNode indexArray[MAX_OPEN_FILES];

void printRecord(Record record)
{
  printf("Entry with id : %i, city : %s, name : %s, and surname : %s\n",
         record.id, record.city, record.name, record.surname);
}

void printHashNode(HashNode node)
{
  printf("HashNode with value : %i, and block_num : %i\n", node.value, node.block_num);
}

int hashFunction(int id, int depth)
{
  //id = id * 123456;

  int a;  //3-5
  a = id >> 3;
  a = a & 7;
  
  int b;  //8-9
  b = id >> 8;
  b = b & 3;
  
  int c;  //12-14
  c = id >> 12;
  c = c & 7;

  int cba = (c << 5) | (b << 3) | a;

  int number_of_values = pow(2.0, (double)depth);
  return id % number_of_values;
}


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

  max_records = (BF_BLOCK_SIZE - sizeof(DataHeader)) / sizeof(Record);
  max_hNodes = BF_BLOCK_SIZE / sizeof(HashNode);
  
  return HT_OK;
}

HT_ErrorCode HT_CreateIndex(const char *filename, int depth)
{

  printf("Name given : %s, max depth : %i\n", filename, depth);
  CALL_BF(BF_CreateFile(filename));

  BF_Block *block;
  BF_Block_Init(&block);

  Entry empty;
  empty.header.local_depth = depth-1;
  empty.header.size = 0;

  int id;
  HT_OpenIndex(filename, &id);
  int fd = indexArray[id].fd;
  
  HashEntry hashEntry;
  int hashN = pow(2.0, (double) depth);
  hashEntry.header.size = hashN;
  for(int i = 0; i < hashN; i++)hashEntry.hashNode[i].value = i;

  // create first block for info
  CALL_BF(BF_AllocateBlock(fd, block));
  char *data = BF_Block_GetData(block);
  memcpy(data, &depth, sizeof(int));
  BF_Block_SetDirty(block);
  CALL_BF(BF_UnpinBlock(block));

  int blockN;

  CALL_BF(BF_AllocateBlock(fd, block));
  CALL_BF(BF_UnpinBlock(block));

  for(int i = 0; i < hashN; i++){

    if(i % 2 == 0){
    BF_GetBlockCounter(fd, &blockN);
    CALL_BF(BF_AllocateBlock(fd, block));
    data = BF_Block_GetData(block);
    memcpy(data, &empty, sizeof(Entry));
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    }
    hashEntry.hashNode[i].block_num = blockN;
  }

  printf("hashEntry size = %i\n", hashEntry.header.size);
  for(int i=0; i < hashN; i++){
    printHashNode(hashEntry.hashNode[i]);
  }

  // create second block for hashing
  BF_GetBlock(fd, 1, block);
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

  //get depth
  int depth;

  CALL_BF(BF_GetBlock(fd, 0, block));
  char *data = BF_Block_GetData(block);
  memcpy(&depth, data, sizeof(int));
  CALL_BF(BF_UnpinBlock(block));

  BF_GetBlockCounter(fd, &blockN);
  int value = hashFunction(record.id, depth);

  HashEntry hashEntry;

  //get hashNodes
  CALL_BF(BF_GetBlock(fd, 1, block));
  data = BF_Block_GetData(block);
  memcpy(&hashEntry, data, sizeof(HashEntry));
  CALL_BF(BF_UnpinBlock(block));

  //number of hashNodes
  int hashN = pow(2.0, (double) depth);

  //find bucket
  int pos;
  for(pos = 0; pos < hashN; pos++)if(hashEntry.hashNode[pos].value == value)break;
  blockN = hashEntry.hashNode[pos].block_num;

  
  BF_GetBlock(fd, blockN, block);
  data = BF_Block_GetData(block);
  memcpy(&entry, data, sizeof(Entry));
  CALL_BF(BF_UnpinBlock(block));

  
  //number of Records in the block
  int size = entry.header.size;
  int local_depth = entry.header.local_depth;

  //check for available space
  if(size >= max_records){
    //space needs to be allocated
    printf("Overflow\n");
  
    //d' < d aka local_depth < global_depth
    if(local_depth < depth){
      // printf("local = %i, global = %i\n", local_depth, depth);
      // printf("d' < d\n");


      int dif = depth - local_depth;
      // printf("dif = %i\n", dif);
      int numOfHashes = pow(2.0, (double) dif);
      // printf("numOfHashes = %i\n", numOfHashes);
      int first;
      for(first = 0; first < hashEntry.header.size; first++)if(hashEntry.hashNode[first].block_num == blockN)break;
      
      // printf("first = %i\n", first);
      int half = first + numOfHashes/2 -1;
      int end = first + numOfHashes - 1;
      // printf("half = %i, end = %i\n", half, end);

      int blockNew;
      BF_GetBlockCounter(fd, &blockNew);
      CALL_BF(BF_AllocateBlock(fd, block));
      CALL_BF(BF_UnpinBlock(block));


      Entry old, new;
      new.header.local_depth = local_depth + 1;
      new.header.size = 0;

      old = entry;
      old.header.local_depth ++;
      old.header.size = 0;

      for(int i = half+1; i <= end; i++)hashEntry.hashNode[i].block_num = blockNew;
      
      CALL_BF(BF_GetBlock(fd, 1, block));
      data = BF_Block_GetData(block);
      memcpy(data, &hashEntry, sizeof(HashEntry));
      BF_Block_SetDirty(block);
      CALL_BF(BF_UnpinBlock(block));

      for(int i = first; i <= end; i++)printHashNode(hashEntry.hashNode[i]);

      for(int i = 0; i < entry.header.size; i++){
        if(hashFunction(entry.record[i].id, depth) <= half){
          old.record[old.header.size] = entry.record[i];
          old.header.size ++;
        }else{
          new.record[new.header.size] = entry.record[i];
          new.header.size ++;
        }
      }

      if(hashFunction(record.id, depth) <= half){
          old.record[old.header.size] = record;
          old.header.size ++;
        }else{
          new.record[new.header.size] = record;
          new.header.size ++;
        }


      //write old
      CALL_BF(BF_GetBlock(fd, blockN, block));
      data = BF_Block_GetData(block);
      memcpy(data, &old, sizeof(Entry));
      BF_Block_SetDirty(block);
      CALL_BF(BF_UnpinBlock(block));

      //write new
      CALL_BF(BF_GetBlock(fd, blockNew, block));
      data = BF_Block_GetData(block);
      memcpy(data, &new, sizeof(Entry));
      BF_Block_SetDirty(block);
      CALL_BF(BF_UnpinBlock(block));

    }else {
      //double table size
      
    }

    return HT_OK;  

  }
  
  CALL_BF(BF_GetBlock(fd, blockN, block));

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

  //get depth
  int depth;

  CALL_BF(BF_GetBlock(fd, 0, block));
  char *data = BF_Block_GetData(block);
  memcpy(&depth, data, sizeof(int));
  CALL_BF(BF_UnpinBlock(block));

  //number of hash values
  int hashN = pow(2.0, (double) depth);

  //get hashNodes
  CALL_BF(BF_GetBlock(fd, 1, block));
  data = BF_Block_GetData(block);
  memcpy(&hashEntry, data, sizeof(HashEntry));
  CALL_BF(BF_UnpinBlock(block));

  //if id == NULL -> print all entries
  if(id == NULL){

    
    //for every hash value
    for(int i = 0; i < hashEntry.header.size; i++){

      //get data block
      blockN = hashEntry.hashNode[i].block_num;
      printf("block_num = %i\n", blockN);
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
  int value = hashFunction((*id), depth);

  //find data block_num
  int pos;
  for(pos = 0; pos < hashN; pos++)if(hashEntry.hashNode[pos].value == value)break;
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