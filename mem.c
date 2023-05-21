#include <stdio.h>
#include <assert.h>
#include "mem.h"

static byte mem[MEMSIZE];
word reg[REGSIZE];

void b_write(Adress adr, byte b) {
	mem[adr] = b;
}

byte b_read(Adress adr) {
	return mem[adr];
}

void w_write(Adress adr, word w) {
	if(adr < 8) {
		reg[adr] = w;
		return;
	}
	mem[adr] = w;
	mem[adr+1] = w >> 8;
}

word w_read(Adress adr) {
	word w = mem[adr+1];
	w = w << 8;
	w = w | mem[adr];
	return w & 0xFFFF;
}

void load_file(const char * filename) {
}

void load_data() {
	int n;
	byte b;
	
	while(scanf("%x %x", &pc, &n) != EOF) {
		int i;
		for(i = 0; i < n; i ++) {
			scanf("%hhx", &b);
			b_write(pc + i, b);
		}
	}
}

void mem_dump(Adress adr, int size) {
	printf("%04x %04x\n", adr, size);
	int i;
	for(i = 0; i < size; i += 2) {
		printf("%06o: %06o %04x\n", adr+i, w_read(adr+i), w_read(adr+i));
	}
}
		
void reg_dump() {
	int i;
	for(i = 0; i < 8; i++) {
		printf("reg%d:%o ", i, reg[i]);
	}
}

void mem_clear() {
	memset(mem, 0, MEMSIZE);
}