#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

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

int compareFGPCT(const void *a, const void *b);
int compareFGPCT(const void *a, const void *b){
    const Record* recordA = (const Record*) a;
    const Record* recordB = (const Record*) b;
    // Compare based on FG_PCT_home field
    if (recordA->FG_PCT_home < recordB->FG_PCT_home) return -1;
    if (recordA->FG_PCT_home > recordB->FG_PCT_home) return 1;
    return 0;
}

int main() {
    FILE *file = fopen("C:/Users/nsupr/OneDrive/Documents/Ntu docs/Y4S1/CZ4031/Project 1 B+Tree/games.txt","r");
    if (file == NULL) {
        perror("Error opening file: ");
    }

    //Count the number of lines 
    int record_count=-1;    
    char c;
    do{
        c=fgetc(file);
        if(c=='\n') record_count++;

    }while(c!=EOF);
    
    Record* Database=malloc(sizeof(Record)*record_count);

    file = fopen("C:/Users/nsupr/OneDrive/Documents/Ntu docs/Y4S1/CZ4031/Project 1 B+Tree/games.txt","r");
    //Copying file to database
    if (file != NULL) {
        char* record=malloc(sizeof(char)*110);
        int index=-1;
        while(fgets(record,110,file)){
            if(index!=-1){
                //printf("%s",record);
                //Database[index]=(Record*)malloc(sizeof(Record));
                
                float FT_PCT,FG_PCT,FG3_PCT;
                

                sscanf(record,"%2hhu/%2hhu/%4hu %u %hu %f %f %f %hhu %hhu %hhu",
                    &Database[index].day,
                    &Database[index].month,
                    &Database[index].year,
                    &Database[index].TEAM_ID_home,
                    &Database[index].PTS_home,
                    &FG_PCT,
                    &FT_PCT,
                    &FG3_PCT,
                    &Database[index].AST_home,
                    &Database[index].REB_home,
                    &Database[index].HOME_TEAM_WINS);

                FG_PCT*=1000;
                FT_PCT*=1000;
                FG3_PCT*=1000;
                Database[index].FG_PCT_home=(int)FG_PCT;
                Database[index].FT_PCT_home=(int)FT_PCT;
                Database[index].FG3_PCT_home=(int)FG3_PCT;
               
            }
            index++;
        }
        fclose(file);

        printf("%d",index);
        printf("Printing first record ");
        printf("FG_PCT: %hu\n",Database[0].FG_PCT_home);
        qsort(Database,record_count,sizeof(Record),compareFGPCT);
        printf("Printing first sorted record");
        printf(" FG_PCT: %hu\n",Database[0].FG_PCT_home);

        printf("First 10 FG_PCT: \n");
        for (int i = 0; i < 100; i++) {
            printf("FG_PCT: %hu\n", Database[i].FG_PCT_home);
        }

        FILE *fp;
        fp = fopen("FGPCTHomeCol.CSV", "r");

        if(fp == NULL){
            printf("That file couldn't be opened.");
            system("pause");
            return 1;
        }

        fprintf(fp, "6,7,8,9\n10,11,12,13");
        for (int j = 0; j < record_count + 1; j++) {
            fprintf(fp, Database[j].FG_PCT_home);
        }

        fclose(fp);
        fp = 0;
        system("pause");
        return 0;
    
    

        //Sorted records in block 

        //Simulate accesses with a block as a unit



        const int blockSize = 400;
        const int recordSize = sizeof(Record);
        const int recordsPerBlock = blockSize/recordSize;
        const int blockQuantity = ceil(record_count/(blockSize/recordsPerBlock));
        

        printf("Total number of records: %d\n",record_count); //record_count = 26651 Records
        printf("Size of a record: %d\n",recordSize);
        printf("Number of records stored in a block: %d\n",recordsPerBlock);
        printf("Number of blocks for storing the data: %d\n",blockQuantity);
        

        Record** Disk=malloc(sizeof(Record*)*blockQuantity);

        int row=0;
        for(int block=0;block<blockQuantity;block++){
            Disk[block]=malloc(sizeof(Record)*recordsPerBlock);
            int block_volume = 0;
            int location_on_block = 0;
            while(block_volume<=400)
            {
                Disk[block][0] = Database[row++];
                location_on_block++;
                block_volume+=recordSize;
            }
            // Insert record into the block
            

            
        }

        

    }
    return 0;
}

