#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

int blockSize; //400 Bytes
int recordSize;
int recordsPerBlock;
int blockQuantity;
const int N=38;
typedef struct{
    uint32_t TEAM_ID_home; 
    uint16_t PTS_home; 
    uint16_t FG_PCT_home;
    uint16_t FG3_PCT_home; 
    uint16_t FT_PCT_home;
    uint8_t year; 
    uint8_t month;
    uint8_t day;
    uint8_t AST_home; 
    uint8_t REB_home; 
    uint8_t HOME_TEAM_WINS; 
    uint16_t Datablock;
}Record; 

typedef struct TreeNode{
    struct TreeNode* parent; //8 bytes
    void* pointer[N+1]; //8*(N+1)
    uint16_t keys[N]; //2*N
    uint8_t isLeaf; //1
    uint8_t num_keys; //1
}TreeNode; //N==38

typedef struct Bucket{
    void* sameKeyPointer[48]; //bucket of duplicate pointers
    struct Bucket* next; 
    uint8_t size;
}Bucket;


typedef struct QNode{
    TreeNode* node;
    struct QNode* next;
}QNode;
typedef struct Q{
    int size;
    QNode* head;
    QNode* tail;
}Queue;


void removeBucket(TreeNode* leaf, int index){
    Bucket* temp = leaf->pointer[index];
    while(temp->next!=NULL)
    {
        Bucket* temp2 = temp;
        temp = temp->next;
        temp2=NULL;
    }
    return;
}

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
                float FT_PCT,FG_PCT,FG3_PCT;
               
                sscanf(record,"%2hhu/%2hhu/20%2hhu %u %hu %f %f %f %hhu %hhu %hhu",
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
                 
                Disk[block][rec].FG_PCT_home=round(FG_PCT);
                Disk[block][rec].FT_PCT_home=round(FT_PCT);
                Disk[block][rec].FG3_PCT_home=round(FG3_PCT);
                Disk[block][rec].Datablock=block;

            }
        }
    }
    fclose(file);


    printf("Print size of disk: %d\n",blockSize*blockQuantity);

    //////////////////////////////////////////////////////////////////
    
    return Disk;
}

//////////////////////////////////////////////////////////////////

TreeNode* root=NULL; //There initially does not exist a B+ Tree

int levels(); 
void displayRecord(Record* pointer);
TreeNode* findLeaf(uint16_t key);
void find(uint16_t lowerBound, uint16_t upperBound);
void insertIntoLeaf(TreeNode* leaf, uint16_t key, void* pointer,int insertionPoint);
void insertIntoNonLeaf(TreeNode* node, int left_index, int key, TreeNode* right);
int getLeftIndex(TreeNode* parent, TreeNode* left);
void insertIntoParent(TreeNode* left, int key, TreeNode* right);
void splitNonLeafInsertion(TreeNode* old_node, int left_index, uint16_t key, TreeNode* right);
void splitLeafInsertion(TreeNode* leaf,uint16_t key, void* pointer,int insertionPoint);
void insert(uint16_t key,void* pointer);
void brute_force(Record** Disk,float lowerBound, float upperBound);




int levels(){
    int level=1;
    TreeNode* tmp = root;
    while(!tmp->isLeaf){
        tmp=tmp->pointer[0];
        level++;
    }
    return level;
}

void displayRecord(Record* pointer){
    printf("%2hhu/%2hhu/20%2hhu %u %hu %f %f %f %hhu %hhu %hhu\n",
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


TreeNode* findLeaf(uint16_t key){ //Converted uint16_t to float
    if(root==NULL) return NULL;

    TreeNode* TraversalNode=root;
    while(!TraversalNode->isLeaf){
        printf("searching for leaf\n");
        for(int j=0;j<TraversalNode->num_keys;j++)
            printf("%d ",TraversalNode->keys[j]);
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


void find(uint16_t lowerBound, uint16_t upperBound){

    TreeNode* leaf=findLeaf(lowerBound);

    int i;
    for(i=0;i<leaf->num_keys;i++){
        if(leaf->keys[i]>=lowerBound && leaf->keys[i]<=upperBound) //Find the first key that satisfies this
            break;
    }

    int num_records=0;
    float sum_of_FG3=0;
    int data_blocks_accessed=0;
    int index_nodes_accessed=levels();
    int* datablocks = malloc(sizeof(int));
    
    while(i<=leaf->num_keys && (float)leaf->keys[i]/1000<=upperBound){
        Bucket* buck=leaf->pointer[i];
        num_records+=buck->size;
        for(int j=0;j<buck->size;j++){
            Record* rec = buck->sameKeyPointer[j];
            int k;
            for(k=0;k<data_blocks_accessed;k++){
                if(rec->Datablock==datablocks[k])
                    break;
            }
            if (k==data_blocks_accessed)
                datablocks=realloc(datablocks,sizeof(int)*(++data_blocks_accessed));
                datablocks[data_blocks_accessed-1]=rec->Datablock;
            sum_of_FG3+=(float)rec->FG3_PCT_home/1000;
            displayRecord(rec);
        }
        i++;
        if(i>leaf->num_keys && (float)leaf->keys[N-1]/1000<upperBound && lowerBound!=upperBound){
            leaf=leaf->pointer[N]; //Access the leaf to the right
            index_nodes_accessed++;
        }
    }
    
    printf("The number of index nodes that the process accesses is %d\n",index_nodes_accessed);
    printf("The number of data block accessed is %d\n", data_blocks_accessed);
    printf("The average of FG3_PCT_home is %f\n", (float)sum_of_FG3/num_records);
}


void insertIntoLeaf(TreeNode* leaf, uint16_t key, void* pointer, int insertionPoint){
    // printf("Printing keys and insertion index: %d and %d ",leaf->num_keys,insertionPoint);
    for(int i=leaf->num_keys;i>insertionPoint;i--){
        leaf->keys[i]=leaf->keys[i-1];
        leaf->pointer[i]=leaf->pointer[i-1];
    }
    leaf->keys[insertionPoint]=key;
    leaf->pointer[insertionPoint]=pointer; 
    leaf->num_keys++;
    return;
}


void insertIntoNonLeaf(TreeNode* node, int left_index, int key, TreeNode* right){
    for (int i=node->num_keys;i>left_index;i--){
        node->pointer[i+1]=node->pointer[i]; 
        node->keys[i]=node->keys[i-1];
    }
    node->pointer[left_index+1]=right;
    node->keys[left_index]=key;
    node->num_keys++;
}



int getLeftIndex(TreeNode* parent, TreeNode* left){
    int left_index=0;
    while(left_index <= parent->num_keys && parent->pointer[left_index]!=left)
        left_index++;
    return left_index;
}

void insertIntoParent(TreeNode* left, int key, TreeNode* right){
    int left_index;
    TreeNode* parent=left->parent;
    if(parent==NULL){
        TreeNode* newRoot=malloc(sizeof(TreeNode));
        newRoot->parent=NULL;
        newRoot->pointer[0]=left;
        newRoot->pointer[1] = right;
        newRoot->keys[0]=key;
        newRoot->isLeaf=0;
        newRoot->num_keys=1;
        left->parent = newRoot;
        right->parent = newRoot;
        root=newRoot;
        return;
    }
    left_index=getLeftIndex(parent,left);
    printf("Left index: %d\n",left_index);
    if(parent->num_keys<N){
        insertIntoNonLeaf(parent,left_index,key,right);
        return;
    }
    splitNonLeafInsertion(parent,left_index,key,right);
}

//old node is parent 
void splitNonLeafInsertion(TreeNode* old_node, int left_index, uint16_t key, TreeNode* right){
    TreeNode* newNode=malloc(sizeof(TreeNode));
    newNode->isLeaf=0;
    newNode->num_keys=0;
    newNode->parent=old_node->parent;
    int split=ceil((double)N/2)+1; //Number of pointers on the left hand side
    printf("Split %d",split);

    int i=0,j=0;

    uint16_t tmp_keys[N+1];
    TreeNode* tmp_pointers[N+2];
    

    while(j<N+2){
        if(left_index+1==i)
            tmp_pointers[j++]=right;
        tmp_pointers[j++]=old_node->pointer[i++];
    }
    i=0;
    j=0;
    while(j<N+1){
        if(left_index==i)
            tmp_keys[j++]=key;
        tmp_keys[j++]=old_node->keys[i++];
    }
    
    printf("leaf:");
    for(i=0;i<N+1;i++){
        printf("%d ",tmp_keys[i]);
    }
    ////////////////////////////////////////////////////////////////////

    for(i=0;i<split-1;i++){
        old_node->pointer[i]=tmp_pointers[i];
        old_node->keys[i]=tmp_keys[i];
    }
    old_node->num_keys=split-1;
    old_node->pointer[split-1]=tmp_pointers[split-1];
    int k_prime=tmp_keys[split-1];

    for(i=split,j=0;i<N+1;i++,j++){
        newNode->pointer[j]=tmp_pointers[i];
        newNode->keys[j]=tmp_keys[i];
        newNode->num_keys++;
    }
    newNode->pointer[newNode->num_keys]=tmp_pointers[N+1];
    
    ////////////////////////////////////////////////////////////////////
    
    TreeNode* child;
    for(i=0;i<=newNode->num_keys;i++){
        child=newNode->pointer[i];
        child->parent=newNode;
    }
    
    insertIntoParent(old_node,k_prime,newNode);

    
}


void splitLeafInsertion(TreeNode* leaf,uint16_t key,void* pointer,int insertionPoint){
    TreeNode* newLeaf=malloc(sizeof(TreeNode));
    newLeaf->isLeaf=1;
    newLeaf->num_keys=0;
    newLeaf->parent=leaf->parent;
    
    
    int split=ceil((double)(N+1)/2);
    int i=0,j=0;

    uint16_t tmp_key[N+1];
    Bucket* tmp_buckets[N+1];

    // printf("Insertion point: %d ",insertionPoint);
    while(i<N+1){
        if(insertionPoint==i){
            tmp_key[i]=key;
            tmp_buckets[i]=pointer;
            i++;
        }
        else{
            tmp_key[i]=leaf->keys[j];
            tmp_buckets[i]=leaf->pointer[j];
            i++;
            j++;
        }
        
    }

    for(i=0;i<split;i++){
        leaf->keys[i]=tmp_key[i];
        leaf->pointer[i]=tmp_buckets[i];
    }
    leaf->num_keys=split;

    for(i=0;i<N+1-split;i++){
        newLeaf->keys[i]=tmp_key[split+i];
        newLeaf->pointer[i]=tmp_buckets[split+i];
        newLeaf->num_keys++;
    }

    newLeaf->pointer[N]=leaf->pointer[N];
    leaf->pointer[N]=newLeaf;
    insertIntoParent(leaf,newLeaf->keys[0],newLeaf);
}


void insert(uint16_t key,void* pointer){
    if(root==NULL){ //If there does note exist a B+ Tree
        Bucket* newBucket=malloc(sizeof(Bucket));
        newBucket->next=NULL;
        newBucket->size=1;
        newBucket->sameKeyPointer[0]=pointer;

        root=malloc(sizeof(TreeNode));
        root->parent=NULL;
        root->pointer[0]=newBucket;
        root->pointer[N]=NULL;
        root->keys[0]=key;
        root->isLeaf=1;
        root->num_keys=1;
        return;
    }
    TreeNode* leaf=findLeaf(key); //Find the location of the leaf to insert into

    printf("Inserting the key %d\n",key);
    printf("Current leaf has %d keys\n",leaf->num_keys);
    for(int i=0;i<leaf->num_keys;i++){
        Bucket* bucket=leaf->pointer[i];
        for(int j=0;j<bucket->size;j++)
            displayRecord(bucket->sameKeyPointer[j]);
    }
        
    printf(" #####################################\n");
    int insertionPoint=0;
    int flag=0;

    while(insertionPoint<leaf->num_keys){
        if (leaf->keys[insertionPoint]==key){
            
            flag=1;
            break;
        }
        else if(leaf->keys[insertionPoint]>key)
            break;
        insertionPoint++;
    }

        

    if(flag==1){
        Bucket* bucket=(Bucket*)leaf->pointer[insertionPoint];
        while(bucket->next!=NULL)
            bucket=bucket->next;
        if(bucket->size==48){
            Bucket* newBucket=malloc(sizeof(Bucket));
            newBucket->next=NULL;
            newBucket->size=1;
            newBucket->sameKeyPointer[0]=pointer;
            bucket->next=newBucket;
        }
        else{
            bucket->sameKeyPointer[bucket->size++]=pointer;
        }
    }
    else{
        Bucket* newBucket=malloc(sizeof(Bucket));
        newBucket->next=NULL;
        newBucket->size=1;
        newBucket->sameKeyPointer[0]=pointer;
        if(leaf->num_keys<N){
            insertIntoLeaf(leaf,key,newBucket,insertionPoint);
        }
        else{
            printf("Split leaf insertion");
            splitLeafInsertion(leaf,key,newBucket,insertionPoint);
        }
    }
}


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

int main(){    
    printf("%d ",sizeof(Record));
    return 0;
    clock_t begin,end;
    double time_spent;
    Record** Disk=DiskAllocation(); //Store records into blocks inside disk

    for(int i=0;i<blockQuantity;i++){
        for(int j=0;j<recordsPerBlock;j++){
            printf(" #####################################\n");
            printf("insertion %d\n",i*20+j+1);
            insert(Disk[i][j].FG_PCT_home,&Disk[i][j]);
            
        }
    }
    
    

    return 0;
    //Insert into B+ Tree
    // for(int i=0;i<blockQuantity;i++){ 
    //     for(int j=0;j<recordsPerBlock;j++){
    //         insert(Disk[i][j].FG_PCT_home,&Disk[i][j]);
    //     }
    // }
    
    // return 0;


    // //////////////////////////////////////////////////////////////////
    // //Retrieve FG_PCT=0.5

    // begin=clock();
    // find(0.5,0.5);
    // end=clock(); 
    // time_spent = ((double) (end - begin))/CLOCKS_PER_SEC;
    // printf("Time taken to retrieve records with key value 0.5 using B+ tree: %lf seconds\n",time_spent);

    // begin=clock();
    // brute_force(Disk,0.5,0.5);
    // end=clock();
    // time_spent=((double) (end - begin))/CLOCKS_PER_SEC;
    // printf("Time taken to retrieve records with key value 0.5 using brute force: %lf seconds\n",time_spent);

    // //////////////////////////////////////////////////////////////////
    // //Retrieve FG_PCT from 0.6 to 1
    // begin=clock();
    // find(0.6,1);
    // end=clock();
    // time_spent=((double) (end - begin))/CLOCKS_PER_SEC;
    // printf("Time taken to retrive records with key values from 0.6 to 1 using B+ tree: %lf seconds\n",time_spent);

    // begin=clock();
    // brute_force(Disk,0.6,1);
    // end=clock();
    // time_spent=((double) (end - begin))/CLOCKS_PER_SEC;
    // printf("Time taken to retrive records with key values from 0.6 to 1 using brute force: %lf seconds\n",time_spent);

    

    // return 0;
}


