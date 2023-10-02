#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

typedef struct{
    uint16_t year; // 12 bits for year (limited to 0-4095)
    uint8_t month; // 4 bits for month (limited to 1-12)
    uint8_t day;
    uint32_t TEAM_ID_home; 
    uint16_t PTS_home; 
    uint16_t FG_PCT_home; // index on b+ tree
    uint16_t FG3_PCT_home; 
    uint16_t FT_PCT_home;
    uint8_t AST_home; 
    uint8_t REB_home; 
    uint8_t HOME_TEAM_WINS; 
}Record; //20 Bytes (+1 Byte for data alignment)

typedef struct{
    Record* records; //400 Bytes
    uint blockSize;
    uint recordPerBlock;
}Block;



int compareFGPCT(const void *a, const void *b);
int compareFGPCT(const void *a, const void *b){
    Record* recordA = *(Record **)a;
    Record* recordB = *(Record **)b;
    // Compare based on FG_PCT_home field
    if (recordA->FG_PCT_home < recordB->FG_PCT_home) return -1;
    if (recordA->FG_PCT_home > recordB->FG_PCT_home) return 1;
    return 0;
}


// Modified program using structures
struct read_output{
    int record_count;
    Record** Database;
};
 
typedef struct read_output read_output;

read_output read_records(char* filename, int record_size){

    read_output out;

    FILE *file = fopen(filename, "r");
    int record_count=-1;    
    char c;
    do{
        c=fgetc(file);
        if(c=='\n') record_count++;
    }while(c!=EOF);
    Record** Database=malloc(sizeof(Record*)*(record_count));
    file = fopen(filename,"r");

    int index = -1;
    if (file != NULL) {
        char* record=malloc(sizeof(char)*110);
        while(fgets(record,110,file)){

            printf("this is where error");
            // printf("this is where error");

            if(index!=-1){
                // printf("%s",record);
                Database[index]=(Record*)malloc(sizeof(Record));
                
                float FT_PCT,FG_PCT,FG3_PCT;
                

                // printf("this is where error");
                sscanf(record,"%2hhu/%2hhu/%4hu %u %hu %f %f %f %hhu %hhu %hhu",
                    &Database[index]->day,
                    &Database[index]->month,
                    &Database[index]->year,
                    &Database[index]->TEAM_ID_home,
                    &Database[index]->PTS_home,
                    &FG_PCT,
                    &FT_PCT,
                    &FG3_PCT,
                    &Database[index]->AST_home,
                    &Database[index]->REB_home,
                    &Database[index]->HOME_TEAM_WINS);
                // printf("this is where error");
                FG_PCT*=1000;
                FT_PCT*=1000;
                FG3_PCT*=1000;
                Database[index]->FG_PCT_home=(int)FG_PCT;
                Database[index]->FT_PCT_home=(int)FT_PCT;
                Database[index]->FG3_PCT_home=(int)FG3_PCT;
               
            }
            index++;
        }
    }
    fclose(file);
    printf("%d",index);
    printf("Printing first record");
    printf("FG_PCT: %hu\n",Database[0]->FG_PCT_home);
    qsort(Database,record_count,sizeof(Record*),compareFGPCT);
    printf("Printing first sorted record");
    printf("FG_PCT: %hu\n",Database[0]->FG_PCT_home);

    out.record_count = record_count;
    out.Database = Database;
    return out;
}


