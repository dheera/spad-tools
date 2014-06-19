// Data reader for 32x32 photon counting array from Politecnico di Milano
// Contact: Dheera Venkatraman <dheera@dheera.net>
//
// Usage: spadcounts myfile.bin
//
// Output:
// ASCII lines formatted as
// [frame number (0-16383N)] [pixel number (0-1024)] [time bin number (0-1024)]
// (may change to binary format default in future)

#include<stdio.h>
#include<string.h>
#include<stddef.h>
#include<stdlib.h>
#include<inttypes.h>

typedef struct {
  uint16_t t[1024];
} spadFrame;

typedef union {
  uint16_t allbits;
  struct {
    unsigned short int coarse:6;
    unsigned short int fine:4;
    unsigned short int zero:6;
  } bits;
  struct {
    unsigned short int byte0:8;
    unsigned short int byte1:8;
  } bytes;
} spadRecord;

typedef union {
  uint16_t allbits[18];
  struct {
    uint16_t dummy0[3];
    unsigned short int stop:4;
    unsigned short int dummy1:4;
    unsigned short int dummy2:4;
    unsigned short int dummy3:4;
    uint16_t dummy4[14];
  } bits;
} spadStop;

int main(int argc, char* argv[]) {
  FILE* infile;
  FILE* outfile;
  int result;
  int i,j,k;
  char c;


  unsigned int pixel_indexes[1024];

  for(i=0;i<1024;i++) {
    if(i%2) {
      pixel_indexes[i] = (0b1111100000 ^ ( (31-(i/64))<<5 ) ) |
                         (0b0000000000 ^ (i/2 & 0b0000011111 ) );
    } else {
      pixel_indexes[i] = (0b1111100000 ^ ( (i+1)/2 & 0b1111100000 ) ) |
                         (0b0000000000 ^ ( (i+1)/2 & 0b0000011111 ) );
    }
  }

  spadRecord raw_records[1024];
  spadStop raw_stop;
  spadFrame current_frame;
  char outfile_name[1024];

  if((infile=fopen(argv[1],"rb"))==NULL) {
    fprintf(stderr,"error: unable to open file for reading: %s\n", argv[1]);
    exit(1);
  }

  sprintf(outfile_name, "%s.out", argv[1]);

  if((outfile=fopen(outfile_name,"wb"))==NULL) {
    fprintf(stderr,"error: unable to open file for writing: %s\n", argv[2]);
    exit(1);
  }

  for(j=0;!feof(infile);j++) {

    result = fread(raw_records, sizeof(spadRecord), 1024, infile);
    if(result<1024) break;
    result = fread(&raw_stop, sizeof(spadStop), 1, infile);
    if(result<1) break;
    
    for(i=0;i<1024;i++) {
      if(raw_records[i].bits.coarse == 0b00111111) {
        current_frame.t[pixel_indexes[i]] = 65535;
      } else {
        current_frame.t[pixel_indexes[i]] =
          (raw_records[i].bits.coarse<<4 | raw_stop.bits.stop)
          - raw_records[i].bits.fine;
      }
    }

    for(i=0;i<1024;i++) {
      if(current_frame.t[i]!=65535) {
        fprintf(outfile, "%d %d %d\n", j, i, current_frame.t[i]);
      }
    }

    if(j%32==0) {
      printf("Reading frame %d ...\r", j);
    }
  }
  printf("done                              \n");

  fclose(infile);
  fclose(outfile);

  exit(0);
  return(0);
}
