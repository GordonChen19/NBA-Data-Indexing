#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

int blockSize; //400 Bytes
int recordSize;
int recordsPerBlock;
int blockQuantity;
const int N = 38;

typedef struct{
    uint16_t year; // 12 bits for year (limited to 0-4095)
    uint8_t month; // 4 bits for month (limited to 1-12)
    uint8_t day;
    uint32_t TEAM_ID_home; 
    uint16_t PTS_home; 
    uint16_t FG_PCT_home;
    uint16_t FG3_PCT_home; 
    uint16_t FT_PCT_home;
    uint8_t AST_home; 
    uint8_t REB_home; 
    uint8_t HOME_TEAM_WINS; 
}Record; //20 Bytes (+1 Byte for data alignment)

typedef struct TreeNode{
    struct TreeNode* parent; //8 bytes
    void* pointer[39]; //8*(N+1)
    uint16_t keys[38]; //2*N
    uint8_t isLeaf; //1
    uint8_t num_keys; //1
}TreeNode; //N==38

typedef struct Bucket{
    void* sameKeyPointer[45]; //bucket of duplicate pointers 
    uint8_t size;
}Bucket;


Record** DiskAllocation();
Record** DiskAllocation() {
    FILE *file = fopen("games.txt","r");

    //Count the number of lines 
    int record_count=-1;    
    char c;
    do{
        c=fgetc(file);
        if(c=='\n') record_count++;

    }while(c!=EOF);

    //////////////////////////////////////////////////////////////////

    //Solution to Part 1
    
    blockSize = 400;
    recordSize = sizeof(Record);
    recordsPerBlock = blockSize/recordSize;
    blockQuantity = ceil(record_count/(blockSize/recordsPerBlock));
    

    printf("Total number of records: %d\n",record_count); //record_count = 26651 Records
    printf("Size of a record: %d\n",recordSize);
    printf("Number of records stored in a block: %d\n",recordsPerBlock);
    printf("Number of blocks for storing the data: %d\n",blockQuantity);

    //////////////////////////////////////////////////////////////////

    //Simulate accesses with a block as a unit

    Record** Disk=malloc(sizeof(Record*)*blockQuantity);

    file=fopen("games.txt","r");
    

    char* record=malloc(sizeof(char)*110);
    fgets(record,110,file); //Ignore the first line of code

    for(int block=0;block<blockQuantity;block++){
        Disk[block]=malloc(sizeof(Record)*recordsPerBlock);
        for(int rec=0;rec<recordsPerBlock;rec++){ //Storing records in each block 
            if(fgets(record,110,file)){
                printf("%s",record);
                float FT_PCT,FG_PCT,FG3_PCT;
                sscanf(record,"%2hhu/%2hhu/%4hu %u %hu %f %f %f %hhu %hhu %hhu",
                    &Disk[block][rec].day,
                    &Disk[block][rec].month,
                    &Disk[block][rec].year,
                    &Disk[block][rec].TEAM_ID_home,
                    &Disk[block][rec].PTS_home,
                    &FG_PCT,
                    &FT_PCT,
                    &FG3_PCT,
                    &Disk[block][rec].AST_home,
                    &Disk[block][rec].REB_home,
                    &Disk[block][rec].HOME_TEAM_WINS);
                FG_PCT*=1000;
                FT_PCT*=1000;
                FG3_PCT*=1000;
                Disk[block][rec].FG_PCT_home=(int)FG_PCT;
                Disk[block][rec].FT_PCT_home=(int)FT_PCT;
                Disk[block][rec].FG3_PCT_home=(int)FG3_PCT;

            }
        }
    }
    fclose(file);

    printf("Printing first record");
    printf("FG_PCT: %hu\n",Disk[0][0].FG_PCT_home);

    printf("Print size of disk: %d\n",blockSize*blockQuantity);

    //////////////////////////////////////////////////////////////////
    
    return Disk;
}

//////////////////////////////////////////////////////////////////

TreeNode* root=NULL; //There initially does not exist a B+ Tree

int levels(); 
int levels(){
    int level=1;
    TreeNode* tmp = root;
    while(!tmp->isLeaf){
        tmp=tmp->pointer[0];
        level++;
    }
    return level;
}

void displayRecord(Record* pointer);
void displayRecord(Record* pointer){
    printf("%2hhu/%2hhu/%4hu %u %hu %f %f %f %hhu %hhu %hhu\n",
        pointer->day,
        pointer->month,
        pointer->year,
        pointer->TEAM_ID_home,
        pointer->PTS_home,
        (float)pointer->FG_PCT_home/1000,
        (float)pointer->FT_PCT_home/1000,
        (float)pointer->FG3_PCT_home/1000,
        pointer->AST_home,
        pointer->REB_home,
        pointer->HOME_TEAM_WINS);
}

TreeNode* findLeaf(uint16_t key);
TreeNode* findLeaf(uint16_t key){
    if(root==NULL) return NULL;

    TreeNode* TraversalNode=root;
    while(!TraversalNode->isLeaf){
        int i=0;
        while(i<TraversalNode->num_keys){
            if(key>=TraversalNode->keys[i])
                i++;
            else
                break;
        } 
        TraversalNode=(TreeNode*)TraversalNode->pointer[i];
    }
    return TraversalNode; //Returns leaf node
}

void find(float lowerBound, float upperBound);
void find(float lowerBound, float upperBound){
    if(root==NULL) return;

    TreeNode* leaf=findLeaf(lowerBound);

    int i;
    for(i=0;i<leaf->num_keys;i++){
        if(leaf->keys[i]>=lowerBound && leaf->keys[i]<=upperBound) //Find the first key that satisfies this
            break;
    }

    int num_records=0;
    int average;
    float sum_of_FG3=0;
    int num_data_blocks=0;
    int index_nodes_accessed=levels();
    if(i==leaf->num_keys)
        return;
    else{
        while(i<=leaf->num_keys && (float)leaf->keys[i]/1000<=upperBound){
            Bucket* buck=leaf->pointer[i];
            num_records+=buck->size;
            for(int j=0;j<buck->size;j++){
                Record* rec = buck->sameKeyPointer[j];
                sum_of_FG3+=(float)rec->FG3_PCT_home/1000;
                displayRecord(rec);
            }
            i++;
            if(i>leaf->num_keys && (float)leaf->keys[N-1]/1000<upperBound && lowerBound!=upperBound){
                leaf=leaf->pointer[N]; //Access the leaf to the right
                index_nodes_accessed++;
            }
        }
    }
}


void createNewRoot(TreeNode*left, uint16_t,TreeNode*right);
void createNewRoot(TreeNode*left,uint16_t key,TreeNode*right){
    TreeNode* root=malloc(sizeof(TreeNode));
    root->parent=NULL;
    root->pointer[0]=left;
    root->pointer[1] = right;
    root->keys[0]=key;
    root->isLeaf=0;
    root->num_keys=1;
    left->parent = root;
    right->parent = root;
}

TreeNode* createNewLeaf();
TreeNode* createNewLeaf(){
    TreeNode* newLeaf=malloc(sizeof(TreeNode));
    newLeaf->isLeaf=1;
    newLeaf->num_keys=0;
    newLeaf->parent=NULL;
    return newLeaf;
}

void insertIntoLeaf(TreeNode* leaf, uint16_t key, void* pointer);
void insertIntoLeaf(TreeNode* leaf, uint16_t key, void* pointer){
    int insertionPoint=0;
    while(insertionPoint<leaf->num_keys && leaf->keys[insertionPoint]<key)
        insertionPoint++; 
    for(int i=leaf->num_keys;i>insertionPoint;i--){
        leaf->keys[i]=leaf->keys[i-1];
        leaf->pointer[i]=leaf->pointer[i-1];
    }
    leaf->keys[insertionPoint]=key;
    leaf->pointer[insertionPoint]=pointer;
    leaf->num_keys++;
}

int getLeftIndex(TreeNode* parent, TreeNode* left);
int getLeftIndex(TreeNode* parent, TreeNode* left){
    int left_index=0;
    while(left_index <= parent->num_keys && parent->pointer[left_index]!=left)
        left_index++;
    return left_index;
}

void insertIntoNode(TreeNode* node, int left_index, int key, TreeNode* right);
void insertIntoNode(TreeNode* node, int left_index, int key, TreeNode* right){
    for (int i=node->num_keys;i>left_index;i--){
        node->pointer[i+1]=node->pointer[i]; 
        node->keys[i]=node->keys[i-1];
    }
    node->pointer[left_index+1]=right;
    node->keys[left_index]=key;
    node->num_keys++;
}

void splitNonLeafInsertion(TreeNode* old_node, int left_index, uint16_t key, TreeNode* right);
void insertIntoParent(TreeNode* left, int key, TreeNode* right);

void insertIntoParent(TreeNode* left, int key, TreeNode* right){
    int left_index;
    TreeNode* parent=left->parent;
    if(parent==NULL){
        createNewRoot(left,key,right);
        return;
    }
    left_index=getLeftIndex(parent,left);
    if(parent->num_keys<N){
        insertIntoNode(parent,left_index,key,right);
        return;
    }
    splitNonLeafInsertion(parent,left_index,key,right);
}


void splitNonLeafInsertion(TreeNode* old_node, int left_index, uint16_t key, TreeNode* right){
    int split=ceil(N/2)+1, k_prime;
    uint16_t tmp_keys[39];
    TreeNode* tmp_pointers[40];
    for(int i=0,j=0;i<old_node->num_keys+1;i++,j++){
        if(j==left_index+1)
            j++;
        tmp_pointers[j]=old_node->pointer[i];
    }
    for(int i=0,j=0;i<old_node->num_keys;i++,j++){
        if(j==left_index)
            j++;
        tmp_keys[j]=old_node->pointer[i];
    }
    tmp_pointers[left_index+1]=right;
    tmp_keys[left_index]=key;

    TreeNode* newNode=malloc(sizeof(TreeNode));
    

    for(int i=0;i<split;i++){
        old_node->pointer[i]=tmp_pointers[i];
        old_node->keys[i]=tmp_pointers[i];
    }

    old_node->num_keys=split;
    old_node->pointer[split]=tmp_pointers[split];
    k_prime = tmp_keys[split];

    newNode->num_keys=0;
    for(int i=split+1,j=0;i<N+1;i++,j++){
        newNode->pointer[j]=tmp_pointers[i];
        newNode->keys[j]=tmp_keys[i];
        newNode->num_keys++;
    }
    newNode->pointer[newNode->num_keys]=tmp_pointers[-1];
    free(tmp_pointers);
    free(tmp_keys);
    newNode->parent =old_node->parent;
    TreeNode* child;
    for(int i=0;i<=newNode->num_keys;i++){
        child=newNode->pointer[i];
        child->parent=newNode;
    }
    insertIntoParent(old_node,k_prime,newNode);

    
}



void splitLeafInsertion(TreeNode* leaf,uint16_t key, void* pointer);
void splitLeafInsertion(TreeNode* leaf,uint16_t key,void* pointer){
    TreeNode* new_leaf=createNewLeaf();
    uint16_t tmp_keys[39];
    TreeNode* tmp_pointers[40];
    int insertion_index=0,split,new_key;

    while(insertion_index<N && leaf->keys[insertion_index]<key){
        insertion_index++;

    }

    for (int i = 0, j = 0; i < leaf->num_keys; i++, j++) {
        if (j == insertion_index) {
            tmp_keys[j] = key;
            tmp_pointers[j] = pointer;
            j++;
        }
        tmp_keys[j] = leaf->keys[i];
        tmp_pointers[j] = leaf->pointer[i];
    }
    split=ceil((N+1)/2);

    for(int i=0;i<N;i++){
        if(i<split){
            leaf->pointer[i]=tmp_pointers[i];
            leaf->keys[i]=tmp_keys[i];
        }
        else
            leaf->pointer[i]=NULL;
    }
    leaf->num_keys=split;
    for(int i=split,j=0;j<N;i++,j++){
        if(j<N+1-split){
            new_leaf->pointer[j]=tmp_pointers[i];
            new_leaf->keys[j]=tmp_pointers[i];
            new_leaf->num_keys++;
        }
        else
            new_leaf->pointer[j]=NULL;
    }
    new_leaf->parent=leaf->parent;
    free(tmp_keys);
    free(tmp_pointers);
    
    new_leaf->pointer[N]=leaf->pointer[N];
    leaf->pointer[N]=new_leaf;

    new_key=new_leaf->keys[0];
    insertIntoParent(leaf,new_key,new_leaf);

}

void insert(uint16_t key,void* pointer);
void insert(uint16_t key,void* pointer){
    if(root==NULL){ //If there does note exist a B+ Tree
        root=malloc(sizeof(TreeNode));
        root->parent=NULL;
        root->pointer[0]=pointer;
        root->keys[0]=key;
        root->isLeaf=1;
        root->num_keys=1;
        return;
    }
    TreeNode* leaf=findLeaf(key); //Find the location of the leaf to insert into
    if(leaf->num_keys<N){
        insertIntoLeaf(leaf,key,pointer);
        return;
    }
    splitLeafInsertion(leaf,key,pointer);
}

void brute_force(Record** Disk,float lowerBound, float upperBound);
void brute_force(Record** Disk, float lowerBound, float upperBound){
    int quantity=0;
    for(int i=0;i<blockQuantity;i++){
        for(int j=0;j<recordsPerBlock;j++){
            if((float)Disk[i][j].FG_PCT_home/1000>=lowerBound && (float)Disk[i][j].FG_PCT_home/1000<=upperBound){
                quantity++;
                displayRecord(&Disk[i][j]);
            }
        }
    }
    printf("Total number of records found in the range %f to %f using the brute force method: %d\n",lowerBound,upperBound,quantity);
}

int get_neighbor_index(TreeNode* leaf);
int get_neighbor_index(TreeNode* leaf){
    int i;
    for (i = 0; i <= leaf->parent->num_keys; i++)
    if (leaf->parent->pointer[i] == leaf)
      return i - 1;
}

TreeNode *removeFromLeaf(TreeNode* leaf, int key, TreeNode* pointer);
TreeNode *removeFromLeaf(TreeNode* leaf, int key, TreeNode* pointer){
    int i = 0;
    int num_pointers;
    
    while (leaf->keys[i] != key){
        i++;
    }
    for (++i; i < leaf->num_keys; i++){
        leaf->keys[i - 1] = leaf->keys[i];
    }
    leaf->num_keys--;

  if (leaf->isLeaf)
    for (i = leaf->num_keys; i < N - 1; i++)
      leaf->pointer[i] = NULL;
  else
    for (i = leaf->num_keys + 1; i < N; i++)
      leaf->pointer[i] = NULL;

  return leaf;

}

TreeNode*redistribute_nodes(TreeNode *root, TreeNode *leaf, TreeNode *neighbor, int neighbor_index,
             int k_prime_index, int k_prime) {
  int i;
  TreeNode *temp;

  if (neighbor_index != -1) {
    if (!leaf->isLeaf)
      leaf->pointer[leaf->num_keys + 1] = leaf->pointer[leaf->num_keys];
    for (i = leaf->num_keys; i > 0; i--) {
      leaf->keys[i] = leaf->keys[i - 1];
      leaf->pointer[i] = leaf->pointer[i - 1];
    }
    if (!leaf->isLeaf) {
      leaf->pointer[0] = neighbor->pointer[neighbor->num_keys];
      temp = (TreeNode *)leaf->pointer[0];
      temp->parent = leaf;
      neighbor->pointer[neighbor->num_keys] = NULL;
      leaf->keys[0] = k_prime;
      leaf->parent->keys[k_prime_index] = neighbor->keys[neighbor->num_keys - 1];
    } else {
      leaf->pointer[0] = neighbor->pointer[neighbor->num_keys - 1];
      neighbor->pointer[neighbor->num_keys - 1] = NULL;
      leaf->keys[0] = neighbor->keys[neighbor->num_keys - 1];
      leaf->parent->keys[k_prime_index] = leaf->keys[0];
    }
  }

  else {
    if (leaf->isLeaf) {
      leaf->keys[leaf->num_keys] = neighbor->keys[0];
      leaf->pointer[leaf->num_keys] = neighbor->pointer[0];
      leaf->parent->keys[k_prime_index] = neighbor->keys[1];
    } else {
      leaf->keys[leaf->num_keys] = k_prime;
      leaf->pointer[leaf->num_keys + 1] = neighbor->pointer[0];
      temp = (TreeNode *)leaf->pointer[leaf->num_keys + 1];
      temp->parent = leaf;
      leaf->parent->keys[k_prime_index] = neighbor->keys[0];
    }
    for (i = 0; i < neighbor->num_keys - 1; i++) {
      neighbor->keys[i] = neighbor->keys[i + 1];
      neighbor->pointer[i] = neighbor->pointer[i + 1];
    }
    if (!leaf->isLeaf)
      neighbor->pointer[i] = neighbor->pointer[i + 1];
  }

  leaf->num_keys++;
  neighbor->num_keys--;

  return root;
             }


void delete_records_less_than_key_from_disk(Record** Disk, float FGPCT_value){
    int index;
    TreeNode* target_leaf = findLeaf(FGPCT_value);
    for (int k = 0; k<target_leaf->num_keys; k++){
        if(FGPCT_value == target_leaf->keys[k]){
            index = k;
            break;
        }
    }
    for (int i = 0; i < blockQuantity; i++){
        if(Disk[i] == target_leaf->pointer[index]){
            break;
        } 
        else{
            for(int j=0; j<recordsPerBlock; j++){
                Disk[i][j].year = NULL;
                Disk[i][j].month = NULL;
                Disk[i][j].day = NULL;
                Disk[i][j].TEAM_ID_home = NULL;
                Disk[i][j].PTS_home = NULL;
                Disk[i][j].FG_PCT_home = NULL;
                Disk[i][j].FG3_PCT_home = NULL;
                Disk[i][j].FT_PCT_home = NULL;
                Disk[i][j].AST_home = NULL;
                Disk[i][j].REB_home = NULL;
                Disk[i][j].HOME_TEAM_WINS = NULL;
            }
        }
    }   
}

int main(){    
    clock_t begin,end;
    double time_spent;
    Record** Disk=DiskAllocation(); //Store records into blocks inside disk

    printf("wtf");

    //Insert into B+ Tree
    for(int i=0;i<blockQuantity;i++){ 
        for(int j=0;j<recordsPerBlock;j++){
            insert(Disk[i][j].FG_PCT_home,&Disk[i][j]);
        }
    }
    

    //////////////////////////////////////////////////////////////////
    //Retrieve FG_PCT=0.5

    begin=clock();
    find(0.5,0.5);
    end=clock(); 
    time_spent = (end-begin)/CLOCKS_PER_SEC;
    printf("Time taken to retrieve records with key value 0.5 using B+ tree: %lf seconds\n",time_spent);

    begin=clock();
    brute_force(Disk,0.5,0.5);
    end=clock();
    time_spent=(end-begin)/CLOCKS_PER_SEC;
    printf("Time taken to retrieve records with key value 0.5 using brute force: %lf seconds\n",time_spent);

    //////////////////////////////////////////////////////////////////
    //Retrieve FG_PCT from 0.6 to 1
    begin=clock();
    find(0.6,1);
    end=clock();
    time_spent=(end-begin)/CLOCKS_PER_SEC;
    printf("Time taken to retrive records with key values from 0.6 to 1 using B+ tree: %lf seconds\n",time_spent);

    begin=clock();
    brute_force(Disk,0.6,1);
    end=clock();
    time_spent=(end-begin)/CLOCKS_PER_SEC;
    printf("Time taken to retrive records with key values from 0.6 to 1 using brute force: %lf seconds\n",time_spent);

    //////////////////////////////////////////////////////////////////
    delete_records_less_than_key_from_disk(Disk, 0.35);
    printf("After deletion:");
    find(0,0.35);
    return 0;
}


