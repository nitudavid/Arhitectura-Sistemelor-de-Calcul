#ifndef _HASHCPU_
#define _HASHCPU_

/**
 * Class GpuHashTable to implement functions
 */
typedef struct Probe {
	int key = 0;
	int value = 0;
}Probe;

class GpuHashTable
{
	public:
		GpuHashTable(int size);
		void reshape(int newSize);

		bool insertBatch(int *keys, int* values, int numKeys);
		int* getBatch(int* key, int numKeys);

		~GpuHashTable();

		Probe *hashTable;
		int totalSize;
		int tempSize;
};

#endif
