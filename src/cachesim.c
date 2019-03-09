/**
* @file cachesim.c
* @author Namir Hassan
*/


#include "cachesim.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

struct configType configuration;
struct traceType trace;

unsigned int accessed = 0;
unsigned int loads = 0;
unsigned int stores = 0;
unsigned int hits = 0;
unsigned int misses = 0;
unsigned int loadHits = 0;
unsigned int storeHits = 0;
unsigned int totalBytesWriteToMM = 0;
unsigned int totalBytesReadFromMM = 0;
unsigned int totalInstructions = 0;
unsigned int missedPenalty = 0;

char *fileFromPath(char *path) {
	char *ssc;
	int l = 0;
	ssc = strstr(path, "/");
	do {
		l = strlen(ssc) + 1;
		path = &path[strlen(path) - l + 2];
		ssc = strstr(path, "/");
	} while (ssc);
	return path;
}
int isPowerOfTwo(int x) {
	if (x == 0)
		return 0;
	while (x != 1) {
		if (x % 2 != 0)
			return 0;
		x = x / 2;
	}
	return 1;
}

void readConfigFile(char *filename) {
	FILE * fp = NULL;
	int count = 0;

	fp = fopen(filename, "r");

	if (fp == NULL) {
		printf(
				"File: %s could not be opened. Please verify existence of the file.\n",
				filename);
		exit(EXIT_FAILURE);
	}

	// There will be six (6) cache configuration values in this config file.

	// Read Line Size
	if (!feof(fp)) {
		fscanf(fp, "%d", &configuration.lineSize);

		// Line size should be non-negative power of 2
		if (!isPowerOfTwo(configuration.lineSize)) {
			printf(
					"line size input value %d is negative or not power of 2. Exiting program ....\n",
					configuration.lineSize);
			exit(EXIT_FAILURE);
		}
		count++;
	}

	//  Read Associativity
	if (!feof(fp)) {
		fscanf(fp, "%d", &configuration.associativity);
		// Associativity should be non-negative power of 2
		if (!isPowerOfTwo(configuration.associativity)) {
			printf(
					"associativity input value %d is negative or not power of 2. Exiting program ....\n",
					configuration.associativity);
			exit(EXIT_FAILURE);
		}
		count++;
	}

	// Read Data size
	if (!feof(fp)) {
		fscanf(fp, "%d", &configuration.dataSize);
		// Data size should be non-negative power of 2
		if (!isPowerOfTwo(configuration.dataSize)) {
			printf(
					"dataSize input value %d is negative or not power of 2. Exiting program ....\n",
					configuration.dataSize);
			exit(EXIT_FAILURE);
		}
		count++;
	}

	// Read Replacement Policy
	if (!feof(fp)) {
		fscanf(fp, "%d", &configuration.replacementPolicy);
		// Replacement Policy: 0 for random placement,  1 for FIFO. Not other value
		if (!((configuration.replacementPolicy == 0)
				|| (configuration.replacementPolicy == 1))) {
			printf(
					"replacementPolicy input value %d is not 0 or  not 1. Exiting program ....\n",
					configuration.replacementPolicy);
			exit(EXIT_FAILURE);
		}
		count++;
	}

	// Read Miss penalty
	if (!feof(fp)) {
		fscanf(fp, "%d", &configuration.missPenalty);
		// Miss penaly may be any positive integer
		if (configuration.missPenalty < 0) {
			printf(
					"missPenalty input value %d is not a positive integer. Exiting program ....\n",
					configuration.missPenalty);
			exit(EXIT_FAILURE);
		}
		count++;
	}

	// Read Write allocate
	if (!feof(fp)) {
		fscanf(fp, "%d", &configuration.writeAllocate);
		// Write allocate should be 0 for no-write-allocate or 1 for write-allocate not other value
		if (!((configuration.writeAllocate == 0)
				|| (configuration.writeAllocate == 1))) {
			printf(
					"writeAllocate input value %d is not 0 or not 1. Exiting program ....\n",
					configuration.writeAllocate);
			exit(EXIT_FAILURE);
		}
		count++;
	}
	// There should be total 6 parameters.
	if (count != 6) {
		printf(
				"Could not read all 6 configuration parameters from the configuration value from %s. Try again ...\n",
				filename);
		exit(EXIT_FAILURE);
	}
	fclose(fp);
}

void printConfiguration() {
	printf("Line size: %u bytes\n", configuration.lineSize);
	printf("Associativity: %u\n", configuration.associativity);
	printf("Data size: %u KB\n", configuration.dataSize);
	printf("Replacement Policy: %u\n", configuration.replacementPolicy);
	printf("Miss penalty: %u cycles\n", configuration.missPenalty);
	printf("Write allocate: %u\n", configuration.writeAllocate);
}

unsigned countBits(unsigned number) {
	return (int) log2(number); // Assume, all our numbers are power of 2.
}
void bit_print(unsigned x) {
	int i;
	int n = sizeof(int) * CHAR_BIT; /* CHAR_BIT = 8 defined in limits.h */
	int mask = 1 << (n - 1); /* place a one in the msb, zeros everywhere else */

	for (i = 0; i < n; i++) {
		putchar(((x & mask) == 0) ? '0' : '1'); /* print char on stdin */
		x <<= 1; /* place the next bit in the msb */

		/* print a space every 4 bits */
		if (i != (n - 1) && (i + 1) % (CHAR_BIT / 2) == 0)
			putchar(' ');
	}
}
void fbit_print(unsigned x, FILE *fp) {

	unsigned i;
	for (i = 1 << 31; i > 0; i = i / 2)
		(x & i) ? fprintf(fp, "1") : fprintf(fp, "0");

}

// Create a Set with required number of empty line block(s)
struct setType * createSet(int numberOfLineBlock) {

	//printf(
	//		"createSet(): Creating a set with line blocks for associativity: %d\n",
	//		numberOfLineBlock);

	if (numberOfLineBlock == 0) {
		printf(
				"createSet(): Full associativity (%d) MUST not come here. Try again ...\n",
				numberOfLineBlock);
		exit(EXIT_FAILURE);
	}

	struct lineType *line = (struct lineType *) malloc(
			numberOfLineBlock * sizeof(struct lineType));

	struct setType *set = (struct setType *) malloc(sizeof(struct setType));

	if ((line == NULL) || (set == NULL)) {
		printf(
				"createSet(): Could not create Line Block and/or Set for the line blocks. Try again ...\n");
		exit(EXIT_FAILURE);
	}

	line->tag = 0;
	line->validBit = 0;
	line->dirtyBit = 0;
	line->accessed = 0;

	set->indexCount = 0;
	set->numberOfLines = numberOfLineBlock;
	set->lineBlock = line;

	return set;
}

int findTag(unsigned int setIndex, unsigned int tag, struct setType *cache) {

	if (cache == NULL) {
		printf("findTag(): Cache MUST not be NULL. Something wrong!!!!\n");
		exit(EXIT_FAILURE);
	}

	if (cache[setIndex].lineBlock == NULL) { // Nothing has been put to this set index yet.
		return -1; // set is empty and it is a Miss.
	}

	// Set index is not empty. It has block(s). Let's assign the head of block to a temporary point for traversing block(s).
	struct lineType *ptr = cache[setIndex].lineBlock;

	if (ptr == NULL) {
		printf(
				"findTag(): Cache line block MUST not be NULL. Something wrong!!!!\n");
		exit(EXIT_FAILURE);
	}
	// indexCount keeps track of how many blocks have values in them. So, only traverse block that has value in it.
	for (int i = 1; i <= cache[setIndex].indexCount; i++) { //loop through the lineBlock which has value assigned earlier. indexCount keeps track of it.
		// if line block is valid and has tag that we are looking for then it is a Hit.
		if ((ptr->validBit == 1) && (ptr->tag == tag)) {
			return i; // Found the tag, so it is a Hit, return the index of the block in the set. We are returning index+1 to avoid conflict with 0.
		} else {
			ptr++; // Go to the next block by incrementing the pointer. pointer will point to the next lineBlock in the set.
		}
	}
	return 0; // set is not empty, but tag is missing. It is a Miss.
}
void assignTagToIndex(unsigned int setIndex, int index, unsigned int tag,
		struct setType *cache) {
	if (cache == NULL) {
		printf(
				"assignTagToIndex(): Cache MUST not be NULL. Something wrong!!!!\n");
		exit(EXIT_FAILURE);
	}

	cache[setIndex].lineBlock[index].tag = tag;
}
// If this function is called, it means, there was a Miss.
// if type is -1, create a new set block, assign the tag to the first line block.
// if type is 0, find and empty line block or use the FIFO to evict one to put the tag
int assignTag(int type, unsigned int setIndex, unsigned int tag,
		struct setType *cache) {
	int index = 0;

	if (cache == NULL) {
		printf("assignTag(): Cache MUST not be NULL. Something wrong!!!!\n");
		exit(EXIT_FAILURE);
	}

	if (type == -1) {
//		printf("assignTag(): Allocating blocks in set: %d....\n",
//				configuration.associativity);

		struct setType *set = createSet(configuration.associativity);
		if (set == NULL) {
			printf("assignTag(): Set MUST not be NULL. Something wrong!!!!\n");
			exit(EXIT_FAILURE);
		}

		struct lineType *lineHead = set->lineBlock;
		if (lineHead == NULL) {
			printf(
					"assignTag(): Line Block MUST not be NULL. Something wrong!!!!\n");
			exit(EXIT_FAILURE);
		}

		lineHead->validBit = 1;
		lineHead->dirtyBit = 0;
		lineHead->tag = tag;
		lineHead->accessed = 0;

		set->indexCount = 1; // This is count
		cache[setIndex] = *set;

	} else if (type == 0) { // Could not find in setIndex, Let's put it in empty block or evicted block.

		int nextBlockToPut = cache[setIndex].indexCount;

		// No empty block. Based on Replacement policy, replace one block.
		if (configuration.associativity == cache[setIndex].indexCount) {

//			printf(
//					"assignTag(): No empty block in set: %d (indexCount:%d, nextBlockToPut:%d)\n",
//					configuration.associativity, cache[setIndex].indexCount,
//					nextBlockToPut);

			// Find out which block to put the tag
			if (configuration.replacementPolicy == 1) { // FIFO
				// FIFO, circularly replacing block. Since 0 index gets populated first, it is the first block to remove when all the blocks are full.
				//cache[setIndex].nextBlockToPut = cache[setIndex].nextBlockToPut % cache[setIndex].numberOfLines;
				nextBlockToPut = cache[setIndex].indexCount
						% cache[setIndex].numberOfLines;
				//printf("FIFO: nextBlockToPut: %d\n", nextBlockToPut);
			} else { // Randomly
				nextBlockToPut = rand() % cache[setIndex].numberOfLines; // Randomly selected the next replacement block index.
				//printf("Randomly: nextBlockToPut: %d\n", nextBlockToPut);
			}

			// We found the block where to put the tag
			//nextBlockToPut = cache[setIndex].nextBlockToPut;
			index = nextBlockToPut;

			// Before evicting, check if dirty bit is set or not. if set then write to Main Memory and then overwrite
			if (cache[setIndex].lineBlock[nextBlockToPut].dirtyBit == 1) {// write to memory
//				printf(
//						"Writing back to the Main Memory and turning off the dirty bit.");
				totalBytesWriteToMM = totalBytesWriteToMM
						+ configuration.lineSize;
				// Reset the dirty bit
				cache[setIndex].lineBlock[nextBlockToPut].dirtyBit = 0;
			}

			cache[setIndex].lineBlock[nextBlockToPut].tag = tag;
			cache[setIndex].lineBlock[nextBlockToPut].validBit = 1;
			cache[setIndex].lineBlock[nextBlockToPut].accessed = 1;

		} else { // There are empty block, nextBlockToPut points to empty block.
//			printf(
//					"assignTag(): There are empty block in set: %d (indexCount:%d, nextBlockToPut:%d)\n",
//					configuration.associativity, cache[setIndex].indexCount,
//					nextBlockToPut);
			cache[setIndex].lineBlock[nextBlockToPut].tag = tag;
			cache[setIndex].lineBlock[nextBlockToPut].validBit = 1;
			cache[setIndex].lineBlock[nextBlockToPut].dirtyBit = 0;
			cache[setIndex].lineBlock[nextBlockToPut].accessed = 1;
			index = nextBlockToPut;
			cache[setIndex].indexCount++;
			//cache[setIndex].nextBlockToPut = nextBlockToPut + 1; // increments to the next index
		}
	}
	return index;
}

void setDirtyBit(unsigned int setIndex, int index, struct setType *cache) {
	if (cache == NULL) {
		printf("setDirtyBit(): Cache MUST not be NULL. Something wrong!!!!\n");
		exit(EXIT_FAILURE);
	}
	cache[setIndex].lineBlock[index].dirtyBit = 1;
}

int main(int argc, char **argv) {
	unsigned int bitsInAssociativity = 0; // valid for direct or full associativity.
	unsigned int bitsInTag = 0;
	unsigned int bitsInSetIndex = 0;
	unsigned int bitsInBlockOffset = 0;

	unsigned int tag = 0;
	unsigned int setIndex = 0;
	unsigned int blockOffset = 0;
	unsigned int numberOfBlocks = 0;

	int index = 0;

	float hitRate = 0.0;
	float missRate = 0.0;
	float amal = 0.0;
	char out[100];

	if (argc != 3) {
		printf("usage: ./cachesim <config file> <trace file>\n");
		exit(EXIT_FAILURE);
	}

	printf("%s, %s, %s\n", argv[0], argv[1], argv[2]);

	// This will create the output file as per output requirements.
	FILE *ofp = NULL;
	sprintf(out, "%s.out", fileFromPath(argv[2]));
	ofp = fopen(out, "w+");
	if (ofp == NULL) {
		printf("File: %s could not be opened for writing!!!\n", out);
		exit(EXIT_FAILURE);
	}

	// This will create test or verification file
	FILE *vfp = NULL;
	sprintf(out, "%s_%s.tout", fileFromPath(argv[1]), fileFromPath(argv[2]));
	vfp = fopen(out, "w+");
	if (vfp == NULL) {
		printf("File: %s could not be opened for writing!!!\n", out);
		exit(EXIT_FAILURE);
	}

	// This will create a file for graph data
	FILE *gfp = NULL;
	gfp = fopen("cachesim_graph_data.csv", "a");
	if (gfp == NULL) {
		printf(
				"File: cachesim_graph_data.csv could not be opened for appending to a file\n");
		exit(EXIT_FAILURE);
	}

	// Read configuration file and stored in a global variable named,  configuration.
	readConfigFile(argv[1]);
	printConfiguration();

	//
	// Setup the Cache
	//
	// Here we will find out the cache fields like, valid bit, tag bits, data bits based on the give configuration parameters.

	// For associativity of 2, 4, or multiple of 2. Not applicable for direct or full associativity. 0 for full associativity and direct.
	if (configuration.associativity > 1) {
		bitsInAssociativity = countBits(configuration.associativity);
		//printf("# of bits for associativity: %d\n", bitsInAssociativity);
	}

	// Line or Block size is in bytes.
	bitsInBlockOffset = countBits(configuration.lineSize);
	//printf("# of bits for Block Offset: %d\n", bitsInBlockOffset);

	// Since data size is in KB, we multiply by 1024. 1KB = 1024 bytes
	numberOfBlocks = configuration.dataSize * 1024 / configuration.lineSize;
	//printf("# of blocks: %d\n", numberOfBlocks);

	// Find the Set index from number of data blocks
	bitsInSetIndex = countBits(numberOfBlocks);

	//
	if (configuration.associativity == 0) { // Full associativity
		bitsInSetIndex = 0; // So, no set index bits. Just one set with blocks. Address bits will be divided into tag bits and block offset bits.
	} else {
		// If associativity is multiple of 2. then set index reduces and tag index increases
		bitsInSetIndex = bitsInSetIndex - bitsInAssociativity;
	}
	//printf("# of bits for Set index: %d\n", bitsInSetIndex);

	// Calculate the tag bits from address bits (32 for this simulation), set index bits and block offset bits.
	bitsInTag = 32 - bitsInSetIndex - bitsInBlockOffset;
	//printf("# of bits for Tag: %d\n", bitsInTag);

	printf("Calculating ... ... ...\n");

	// We have calculated Tag, Set Index, Block offset bits for the given configuration.
	printf(
			"%d bits ==> Tag: %d, Set Index: %d and Block Offset: %d for associativity: %d\n",
			(bitsInTag + bitsInSetIndex + bitsInBlockOffset), bitsInTag,
			bitsInSetIndex, bitsInBlockOffset, configuration.associativity);

	// This points to the Cache.
	struct setType *cache = NULL;

	if (configuration.associativity == 0) { // Full Associativity, i.e. there is only one set, this set will have all the line blocks
		// For full associativity, we are creating all the line blocks in a set.
		//There is not set index. so address bits will be divided in tag bits and block offset bits.
		cache = (struct setType *) malloc(sizeof(struct setType)); // Allocating just one set.

		if (cache == NULL) {
			printf(
					"Could not allocate memory for full-associative set for cache!!! Exiting program.");
			exit(EXIT_FAILURE);
		}

		// Allocating Line blocks for total number of blocks. And assigning to the cache set.
		cache->lineBlock = (struct lineType *) malloc(
				numberOfBlocks * sizeof(struct lineType));

		if (cache->lineBlock == NULL) {
			printf(
					"Could not allocate memory for full-associative line blocks for cache set!!! Exiting program.");
			exit(EXIT_FAILURE);
		}

	} else { // associativity can be direct mapped (one line block in each set, OR multiple line blocks in each set based on associativity given.
		// Here we will create all the set but empty line block. We will create new block(s) in set when program will try to access that set index.
		cache = (struct setType *) malloc(
				pow(2.0, bitsInSetIndex) * sizeof(struct setType)); // bitsInSetIndex reduces i.e Set index size reduces based on N-way associativity. N >=1. direct is 1-way.

		if (cache == NULL) {
			printf(
					"Could not allocate memory for N-way associativity cache set!!! Exiting program.");
			exit(EXIT_FAILURE);
		} else { // Initialize the cache otherwise Seg Fault 11 i.e. uninitialized value access might happen
			for (int i = 0; i < pow(2.0, bitsInSetIndex); i++) {
				cache[i].lineBlock = NULL;
				cache[i].numberOfLines = 0;
				cache[i].indexCount = 0;
			}

		}
	}

	//printf("Set Index Size: %u\n", (unsigned) pow(2.0, bitsInSetIndex));

	//
	// READ Trace File and prepare hits and misses based on each line in this file.
	//

	FILE * fp = NULL;
	unsigned address = 0;

	// Reading a trace file given as a second argument in the command line.
	fp = fopen(argv[2], "r");
	if (fp == NULL)
		exit(EXIT_FAILURE);

	//
	// Read each line in the trace file, process it through the cache and update statistics
	//
	while (!feof(fp)) {

		// Read a line from trace file and save all three different fields for processing
		fscanf(fp, "%c %s %d\n", &trace.accessType, trace.address,
				&trace.islma);
		//printf("READ: %c %s %d\n", trace.accessType, trace.address, trace.islma);

		// We read the address field as string (array of characters), so convert it to hexadecimal
		address = (unsigned) strtol(trace.address, NULL, 16);

		//printf("In Decimal: %c %u %d \n", trace.accessType, address, trace.islma);
		//printf("Address in binary: ");
		//bit_print(address);
		//printf("\n");

		// Bits in address can be logically divide into tag, set index, and block offset bits. So, let's separate each field.
		// Extract tag value based on number of tag bits calculated from configuration.
		tag = address >> (bitsInSetIndex + bitsInBlockOffset);
		//printf("Tag: %d , Bits: ", tag);
		//bit_print(tag);
		//printf("\n");

		// Extract set index value based on number of set index bits calculated from configuration.
		setIndex = ((address << bitsInTag) >> (bitsInTag + bitsInBlockOffset));
		//printf("Set Index: %d , Bits: ", setIndex);
		//bit_print(setIndex);
		//printf("\n");

		// Extract block offset value base on number of block offset bits calculated from configuration.
		blockOffset = (address << (bitsInTag + bitsInSetIndex))
				>> (bitsInTag + bitsInSetIndex);
		//printf("Block Offset: %d , Bits: ", blockOffset);
		//bit_print(blockOffset);
		//printf("\n");

		// From the address bits and the given configuration, we have calculated set index value and tag value.
		// we can now check our cache for Hit or Miss for a Load or Store instruction and using the
		// setIndex and tag. Hits, Misses etc will be used to compute output statistics.

		totalInstructions = totalInstructions + trace.islma; // Counting total number of instructions in the program.
		accessed++; // Counting total number of memory accessed.

		// status keeps hit or miss status after searching the case using set index and tag.
		int status = findTag(setIndex, tag, cache);

		// Hit
		if (status >= 1) { // Based on logics in findTag() function. This is a Hit
			hits++;

			index = status - 1; // Block offset in set.

			// It is a Hit for Load or Read, nothing to be done.
			if (trace.accessType == 'l') {
				loads++;
				loadHits++;

				// This is for testing logs to verify each trace line by hit/miss, type of access, address hex, address decimal, block offset in address, setIndex in address,
				//tag in address, associativity, write allocate, replacement policy, memory write comment, and bit representation of address, setIndex and tag
				fprintf(vfp, " Hit, l:%c %s, %u, %2u, ", trace.accessType,
						trace.address, address, blockOffset);
				fprintf(vfp, "%4d, %d, %d, %d, %d, ", setIndex, tag,
						configuration.associativity,
						configuration.writeAllocate,
						configuration.replacementPolicy);
			}

			// Hit for Store or Write, we need to do something
			if (trace.accessType == 's') { // STORE
				stores++;
				storeHits++;

				// This is for testing logs to verify each trace line by hit/miss, type of access, address hex, address decimal, block offset in address, setIndex in address,
				//tag in address, associativity, write allocate, replacement policy, memory write comment, and bit representation of address, setIndex and tag
				fprintf(vfp, " Hit, s:%c %s, %u, %2u, ", trace.accessType,
						trace.address, address, blockOffset);
				fprintf(vfp, "%4d, %d, %d, %d, %d, ", setIndex, tag,
						configuration.associativity,
						configuration.writeAllocate,
						configuration.replacementPolicy);

				// Find out whether it is no-write-allocate(0) with write-through, write-allocate(1) with write-back
				// Hit  --> write-through       or   write-back
				// Miss --> no-write-allocate   or   write-allocate
				if (configuration.writeAllocate == 0) { // no-write-allocate, We need to consider write-through here.

					// Put data in the Cache and write to Main Memory.
					//index = assignTag(status, setIndex, tag, cache);
					assignTagToIndex(setIndex, index, tag, cache); // we know the line block offset in set from index when there is a hit.
					fprintf(vfp,
							"Putting in Cache at setIndex: %d at block index: %d and writing to the Main Memory. ",
							setIndex, index);

					totalBytesWriteToMM = totalBytesWriteToMM
							+ configuration.lineSize;

				} else { // write-allocate, we need to consider write-back here.

					// Just put data in Cache and turn on dirty-bit because of Hit scenario and write-back policy.
					//index = assignTag(status, setIndex, tag, cache);
					assignTagToIndex(setIndex, index, tag, cache); // In our simulation, we are only replacing tag, not keeping track of data. So, replacing same tag. It would make difference when track data.

					// Setting dirty bit to 1 for write-back
					setDirtyBit(setIndex, index, cache);

					fprintf(vfp,
							"Putting in Cache at setIndex: %d at block index: %d and turned on dirty bit. ",
							setIndex, index);
				}
			}

		} else { // Miss

			misses++;
			missedPenalty = missedPenalty + configuration.missPenalty;

			if (trace.accessType == 'l') { // READ
				loads++;

				// This is for testing logs to verify each trace line by hit/miss, type of access, address hex, address decimal, block offset in address, setIndex in address,
				//tag in address, associativity, write allocate, replacement policy, memory write comment, and bit representation of address, setIndex and tag
				fprintf(vfp, "Miss, l:%c %s, %u, %2u, ", trace.accessType,
						trace.address, address, blockOffset);
				fprintf(vfp, "%4d, %d, %d, %d, %d, ", setIndex, tag,
						configuration.associativity,
						configuration.writeAllocate,
						configuration.replacementPolicy);

				// Missed for a load or read. Since it is a load/read, Bring data from main memory and put the tag in line block in the cache.
				fprintf(vfp, "Reading from Main Memory and assign in cache. ");
				totalBytesReadFromMM = totalBytesReadFromMM
						+ configuration.lineSize;

				index = assignTag(status, setIndex, tag, cache);
			}

			if (trace.accessType == 's') { // STORE
				stores++;

				// This is to verify each trace line by hit/miss, type of access, address hex, address decimal, block offset in address, setIndex in address,
				//tag in address, associativity, write allocate, replacement policy, memory write comment, and bit representation of address, setIndex and tag
				fprintf(vfp, "Miss, s:%c %s, %u, %2u, ", trace.accessType,
						trace.address, address, blockOffset);
				fprintf(vfp, "%4d, %d, %d, %d, %d, ", setIndex, tag,
						configuration.associativity,
						configuration.writeAllocate,
						configuration.replacementPolicy);

				if (configuration.writeAllocate == 0) { // no-write-allocate
					fprintf(vfp,
							"No cache allocation. Directly Writing to the Main Memory. ");

				} else {
					fprintf(vfp,
							"Writing to the Main Memory and then allocate in cache. ");
					index = assignTag(status, setIndex, tag, cache);
				}

				totalBytesWriteToMM = totalBytesWriteToMM
						+ configuration.lineSize;

			}

		}

		fbit_print(address, vfp);
		fprintf(vfp, ", ");
		fbit_print(setIndex, vfp);
		fprintf(vfp, ", ");
		fbit_print(tag, vfp);
		fprintf(vfp, ", ");
		fbit_print(blockOffset, vfp);
		fprintf(vfp, "\n");

	} //While ends here

	fclose(fp);

	//
	// Calculate Stats
	//

	// Check each line statistics
	printf("\n\n");
	printf(
			"Accessed: %d, Hits: %d, Misses: %d, totalBytesReadFromMM: %d, totalBytesWriteToMM: %d\n",
			accessed, hits, misses, totalBytesReadFromMM, totalBytesWriteToMM);
	printf("Accessed: %d, Loads: %d, loadHits: %d, \n", accessed, loads,
			loadHits);
	printf("Accessed: %d, Stores: %d,storeHits: %d\n", accessed, stores,
			storeHits);
	hitRate = (float) hits / accessed;
	fprintf(ofp, "Total Hit Rate: %.2f%% \n", hitRate * 100);
	missRate = 1 - hitRate;
	fprintf(ofp, "Load Hit Rate %.2f%% \n", (float) loadHits / loads * 100);
	fprintf(ofp, "Store Hit Rate %.2f%% \n", (float) storeHits / stores * 100);
	fprintf(ofp, "Total Run TIme: %u cycles\n",
			(totalInstructions * 1
					+ (totalBytesReadFromMM + totalBytesWriteToMM)
							* configuration.missPenalty));
	amal = 1 + missRate * configuration.missPenalty;
	fprintf(ofp, "Average Memory Access Latency: %f\n", amal);

	sprintf(out, "%s, %s, %.2f, %.2f", fileFromPath(argv[1]),
			fileFromPath(argv[2]), hitRate, amal);
	fprintf(gfp, "%s\n", out);

	fclose(ofp);
	fclose(vfp);
	fclose(gfp);

	// Free up memory that were allocated.
	if( cache!= NULL){
		for (int i = 0; i < pow(2.0, bitsInSetIndex); i++) {
			if (cache[i].lineBlock != NULL)
				free(cache[i].lineBlock);
		}
		free(cache);
	}
	printf(" ... ... ... Done.\n");
	printf("\n");
	return 0;
}
