#pragma once
#define pc reg[7]
#define REGSIZE 8
#define MEMSIZE (64*1024)

typedef unsigned char byte;
typedef unsigned short int word;
typedef word Adress;

void load_data();
void load_file(const char * filename);
void b_write(Adress adr, byte b);
byte b_read(Adress adr);
void w_write(Adress adr, word w);
word w_read(Adress adr);
void mem_dump(Adress adr, int size);
void reg_dump();