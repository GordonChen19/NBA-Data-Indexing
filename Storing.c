#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
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
    
    const int blockSize = 400;
    const int recordSize = sizeof(Record);
    const int recordsPerBlock = blockSize/recordSize;
    const int blockQuantity = ceil(record_count/(blockSize/recordsPerBlock));
    

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

    //////////////////////////////////////////////////////////////////
    
    return Disk;
}


int main(){ 
    Record** Disk=DiskAllocation(); //Store records into blocks inside disk
    

}