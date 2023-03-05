#include <stdio.h>
#include <string.h>

#define TLB_SIZE 16
#define NUM_FRAMES 256
#define FRAME_SIZE 256
#define NUM_PAGES 256 
#define PAGE_SIZE 256

typedef struct 
{
  int pg_no;
  int fr_no;
} tlb_node;

char physical_memory[NUM_FRAMES][FRAME_SIZE];
int page_table[NUM_PAGES];
tlb_node TLB[TLB_SIZE];

int num_operations = 0;
int nxt_frame = 0;
int pg_faults = 0;
int TLB_hits = 0;
int nxt_TLB = 0;

int check_TLB(int pg_no) // to check if the desired page gives TLB hit
{
  for(int i=0;i<TLB_SIZE;i++)
  {
    if(TLB[i].pg_no == pg_no)
    {
      return TLB[i].fr_no;
    }
  }
  return -1;
}

void add_TLB(int pg_no, int fr_no) // add the page no., frame no. to TLB
{
  TLB[nxt_TLB].pg_no = pg_no;
  TLB[nxt_TLB].fr_no = fr_no;
  nxt_TLB = (nxt_TLB + 1) % TLB_SIZE;
}

void add_page_table(int pg_no, int fr_no) // add the frame no. to page table at thre respective page no.
{
  page_table[pg_no] = fr_no;
}

int main()
{
  //Try to read BACKING_STORE.bin
  FILE* backing_store = fopen("BACKING_STORE.bin", "rb");
  if(backing_store == 0) 
  { 
    printf("Failed opening BACKING_STORE.bin\n");
    return 1;
  }

  //Try to read addresses.txt
  FILE* address_file = fopen("addresses.txt", "rb");
  if(address_file == 0)
  {
    printf("Failed opening addresses.txt\n");
    return 1;
  }

  //initialize all memory, page table, TLB
  for(int i=0;i<NUM_FRAMES;i++)
  {
    for(int j=0;j<FRAME_SIZE;j++)
    {
      physical_memory[i][j] = -1;
    }
  }
  for(int i=0;i<NUM_PAGES;i++)
  {
    page_table[i] = -1;
  } 
  for(int i=0;i<TLB_SIZE;i++)
  {
    TLB[i].pg_no = -1;
    TLB[i].fr_no = -1;
  }

  char address[10]; // need to read 5 characters and one will be newline character
  printf("%10s\t%10s\t%4s\n", "Virtual_Addr", "Physical_Addr", "Data");

  //read one by one from addresses.txt
  while(fgets(address, 10, address_file) != 0) 
  { 
    //increment total operations
    num_operations++;
    
    //convert the address read into an integer
    int logical_address = 0;
    for(int i=0;i<strlen(address)-2;i++)
    {
        logical_address = ((logical_address * 10) + (address[i] - '0'));
    }

    //get page number and offset from LA
    int pg_no = logical_address >> 8;
    pg_no = pg_no & 0b11111111; // for the last 8 bits
    int offset = logical_address & 0b11111111; // for the last 8 bits

    int fr_no = check_TLB(pg_no); //check TLB for hit
    
    //if TLB miss
    if(fr_no == -1)
    {
      fr_no = page_table[pg_no]; //check page table for hit
      
      //if page fault
      if(fr_no == -1)
      {
        fr_no = (nxt_frame) % NUM_PAGES; //check for the next available frame
        nxt_frame++; // Point to next available frame
        add_page_table(pg_no, fr_no); //add frame no. at the resp page no.
        pg_faults++; // increment the no. of faults
     
        //skip to the required page number through random access
        if(fseek(backing_store, pg_no*PAGE_SIZE, SEEK_SET) != 0) 
        {
          printf("Error seeking in BACKING_STORE\n");
          return 1;
        }
  
        //read 256 i.e size of each frame no. of bytes into resp. physical memory frame no.
        if(fread(physical_memory[fr_no], sizeof(char), FRAME_SIZE, backing_store) == 0) 
        {
          printf("Error reading from BACKING_STORE\n");
          return 1;
        }
      }

      add_TLB(pg_no, fr_no); //add it to the TLB after page fault service time
    }
    else
    { 
      TLB_hits++; // if TLB hit
    }
    
    //get the physical address from the frame number
    int physical_address = fr_no << 8;
    physical_address = physical_address | offset;
       
    //get to the required offset
    char read_value = physical_memory[fr_no][offset]; 
    
    //print data obtained
    printf("%10d\t%10d\t%4d\n", logical_address, physical_address, read_value);  
  }

  //printing summary
  printf("Number of Translated Addresses = %d\n", num_operations);
  printf("Page Faults = %d\n", pg_faults);
  printf("Page Fault Rate(percent) = %.2f\n", ((double)pg_faults/(double)num_operations)*100);
  printf("TLB Hits = %d\n", TLB_hits);
  printf("TLB Hit Rate(percent) = %.2f\n", ((double)TLB_hits/(double)num_operations)*100);

  // Close opened files
  fclose(address_file);
  fclose(backing_store);

  // print the contents of the TLB after operations
  printf("The contents of the TLB are\n");
  printf("%6s %6s\n", "Page", "Frame");
  for(int i=0;i<TLB_SIZE;i++)
  {
    printf("%6d %6d\n", TLB[i].pg_no, TLB[i].fr_no);
  }

  // print the contents of the page table after operations
  printf("The contents of the Page Table are\n");
  printf("%6s %6s\n", "Page", "Frame");
  for(int i=0;i<NUM_PAGES;i++)
  {
    printf("%6d %6d\n", i, page_table[i]);
  }
  
  return 0;
}
