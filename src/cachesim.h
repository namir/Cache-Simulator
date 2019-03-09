#ifndef BIT_PRINT_H_
#define BIT_PRINT_H_

#define THIRTY_TWO_BITS 32
#define TEN_CHARS 10
#define TWENTY_CHARS 20

struct configType {
	int lineSize; // in bytes power of 2.
	int associativity; // 0: full-associative, 1: direct-mapped, 2, 4, 8 .. power of 2.
	int dataSize; // in KB, power of 2.
	int replacementPolicy; // 0: Random, 1: FIFO
	int missPenalty; //
	int writeAllocate; // 0: no-write-allocate + write through, 1: write-allocate + write back
};

struct traceType {
	char accessType; // 'l': Load or Read, 's': Store or Write
	char address[TEN_CHARS + 1];
	int islma; // Instructions since last memory access:
};

// valid bit, dirty bit, Tag, and point to other line block(s) (based on associativity)
struct lineType {
	unsigned int validBit: 1;
	unsigned int dirtyBit: 1;
	unsigned int tag;
	unsigned int accessed; //If this number is max. among all the line blocks in the set. it is candidate for Out for FIFO.
};
struct setType {
	int numberOfLines;
	//int nextBlockToPut; // It keep the index number of the line block to put tag
	int indexCount;
	struct lineType *lineBlock; // Keep the beginning point of the array of lineType blocks
};


void sbit_print(unsigned short x);
void bit_print(unsigned x);

#endif
