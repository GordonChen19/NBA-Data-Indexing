//Simulate accesses with a block as a unit

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "memory_management.h"
#define N 100

// Specifying record format 
// typedef struct{
//     uint16_t year; // 12 bits for year (limited to 0-4095)
//     uint8_t month; // 4 bits for month (limited to 1-12)
//     uint8_t day;
//     uint32_t TEAM_ID_home; 
//     uint16_t PTS_home; 
//     uint16_t FG_PCT_home; // index on b+ tree
//     uint16_t FG3_PCT_home; 
//     uint16_t FT_PCT_home;
//     uint8_t AST_home; 
//     uint8_t REB_home; 
//     uint8_t HOME_TEAM_WINS; 
// }Record; //20 Bytes (+1 Byte for data alignment)

// typedef struct{
//     Record* records; //400 Bytes
// }Block;

// typedef struct node {
//     void **pointers;
//     int *keys;
//     struct node* parent;
//     bool is_leaf;
//     int num_keys;
//     struct node *next;
// } node;
// int param_n = N;
// node *queue = NULL;
// bool verbose_output = false;

// void enqueue(node *new_node);
// node *dequeue(void);
// int height(node *const root);
// int pathToLeaves(node *const root, node* child);
// void printLeaves(node *const root);
// void printTree(node *const root);
// void findAndPrint(node *const root, int key, bool verbose);
// void findAndPrintRange(node *const root, int range1, bool verbose);
// int findRange(node *const root, int key_start, int key_end, bool verbose, int returned_keys[], void *returned_pointers[]);
// node *findLeaf(node *const root, int key, bool verbose);
// record *find(node *root, int key, bool verbose, node **leaf_out);
// int cut(int length);

// node *makeNode(void);
// node *makeLeaf(void);
// int getLeftIndex(node *parent, node *left);
// node *insertIntoLeaf(node *leaf, int key, record *pointer);
// node *insertIntoLeafAfterSplitting(node *root, node *leaf, int key,
//                    record *pointer);
// node *insertIntoNode(node *root, node *parent,
//            int left_index, int key, node *right);
// node *insertIntoNodeAfterSplitting(node *root, node *parent,
//                    int left_index,
//                    int key, node *right);
// node *insertIntoParent(node *root, node *left, int key, node *right);
// node *insertIntoNewRoot(node *left, int key, node *right);
// node *startNewTree(int key, record *pointer);
// node *insert(node *root, int key, int value);


int main() {
    const int blockSize = 400; //400 bytes
    const int recordSize = sizeof(Record); // 20 per records

    printf("READING RECORDS INTO THE DATABASE\n");
    
    read_output read_output = read_records("games copy.txt", recordSize);

    Record **Database = read_output.Database;
    int record_count = read_output.record_count;

    const int recordsPerBlock = blockSize/recordSize; // 20 records per block
    const int blockQuantity = ceil(record_count/recordsPerBlock); // a lot
    

    printf("NUMBER OF RECORDS: %d \n", record_count);
    printf("SIZE OF RECORDS: %d \n", recordSize);
    printf("NUMBER OF RECORDS STORED IN A BLOCK: %d \n", recordsPerBlock);
    printf("NUMBER OF BLOCKS: %d \n", blockQuantity);



    Block* Disk = malloc(sizeof(Block)*blockQuantity);
    int row = 0;
    for(int block=0;block<blockQuantity;block++)
    {
        Disk[block].records=malloc(sizeof(Record)*recordsPerBlock);
        int block_volume = 0;
        int location_on_block = 0;
        while(block_volume<=400)
        {
            Disk[block].records[0] = *Database[row];
            row++;
            location_on_block++;
            block_volume+=recordSize;
        }
        // Insert record into the block
        // 
    }
    
    return 0;
}


