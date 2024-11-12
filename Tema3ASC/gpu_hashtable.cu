#include <iostream>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctime>
#include <sstream>
#include <string>
#include "test_map.hpp"
#include "gpu_hashtable.hpp"

using namespace std;

/*
Allocate CUDA memory only through glbGpuAllocator
cudaMalloc -> glbGpuAllocator->_cudaMalloc
cudaMallocManaged -> glbGpuAllocator->_cudaMallocManaged
cudaFree -> glbGpuAllocator->_cudaFree
*/

/******** Kernel Functions ********/

//Function that performs reshape redistributing the elements of a hashTable into a new hashTable with a different size
__global__ void reshapeKernel(Probe* oldhashTable, Probe* newhashTable, int oldSize, int newSize) {
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < oldSize) {
        if (oldhashTable[idx].key != KEY_INVALID) {
            uint32_t key = oldhashTable[idx].key;
			//Hash function
            uint32_t hash = key % newSize;

            //Perform linear probing to find an empty slot
            while (atomicCAS((unsigned int*)&newhashTable[hash].key, KEY_INVALID, key) != KEY_INVALID) {
                hash = (hash + 1) % newSize;
            }

            //Copy the value to the new hash table
            atomicExch((unsigned int*)&newhashTable[hash].value, oldhashTable[idx].value);
        }
    }
}

//Function that inserts a batch of key and value into a hashTable
__global__ void insertBatchKernel(Probe* hashTable, int* keys, int* values, int numKeys, int size, int* tempSize) {
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < numKeys) {
        int key = keys[idx];
        int value = values[idx];
		//Hash function
        int hash = key % size;

        //Perform linear probing to find an empty slot or the matching key
        while (atomicCAS((unsigned int*)&hashTable[hash].key, 0, key) != 0 && hashTable[hash].key != key) {
            hash = (hash + 1) % size;
        }

        //Update the value when we find an empty slot or the matching key
        atomicExch(&hashTable[hash].value, value);

        //Increment the current size if its a new key
        if (hashTable[hash].key == key) {
            atomicAdd(tempSize, 1);
        }
    }
}

//Function that returns a batch of values from a hashTable
__global__ void getBatchKernel(Probe* hashTable, int* keys, int* values, int numKeys, int size) {
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < numKeys) {
        int key = keys[idx];
		//Hash function
        int hash = key % size;

        //Perform linear probing to find the matching key or an empty slot
        while (hashTable[hash].key != key && hashTable[hash].key != 0) {
            hash = (hash + 1) % size;
        }

        //If the key is found, retrieve the value
        if (hashTable[hash].key == key) {
            atomicExch(&(values[idx]), hashTable[hash].value);
        }
    }
}



/**
 * Function constructor GpuHashTable
 * Performs init
 * Example on using wrapper allocators _cudaMalloc and _cudaFree
 */
GpuHashTable::GpuHashTable(int size) 
	: totalSize(size), tempSize(0)
{
	Probe* hashTable;
	glbGpuAllocator->_cudaMallocManaged((void**)&hashTable, (int)(size * sizeof(Probe)));
	cudaMemset((void*)hashTable, 0, (int)(size * sizeof(Probe)));
	this->hashTable = hashTable;
}

/**
 * Function desctructor GpuHashTable
 */
GpuHashTable::~GpuHashTable() {
	glbGpuAllocator->_cudaFree((void*)hashTable);
}

/**
 * Function reshape
 * Performs resize of the hashtable based on load factor
 */
void GpuHashTable::reshape(int numBucketsReshape) {

	//Creating the new hashTable, allocating memory and initializing it with 0
	Probe* newHashTable;
	glbGpuAllocator->_cudaMallocManaged((void**)&newHashTable, (int)(numBucketsReshape * sizeof(Probe)));
	cudaMemset((void*)newHashTable, 0, (int)(numBucketsReshape * sizeof(Probe)));

    //Calculate the number of blocks based on the number of keys and available threads
	int blocks, threads;
    cudaDeviceProp DeviceProp;
    cudaGetDeviceProperties(&DeviceProp, 0);
    threads = DeviceProp.maxThreadsPerBlock;
    if (totalSize % threads == 0) {
        blocks = totalSize / threads;
    }
    else {
        blocks = totalSize / threads + 1;
    }

	//Transfering the old hashTable to the new hashTable using the reshape function
	reshapeKernel<<<blocks, threads>>>(hashTable, newHashTable, totalSize, numBucketsReshape);
	//Ensuring that all processes are completed before the next thread continues executing the next operation
	cudaDeviceSynchronize();

	//Free-ing the old hashTable and reseting the variables
	glbGpuAllocator->_cudaFree((void*)hashTable);
	hashTable = newHashTable;
	totalSize = numBucketsReshape;
}

/**
 * Function insertBatch
 * Inserts a batch of key:value, using GPU and wrapper allocators
 */
bool GpuHashTable::insertBatch(int *keys, int* values, int numKeys) {
	
	//Initializing the new keys and values, allocating memory for them, and copy-ing them from the Host to the Device
	int* newKeys = 0;
	int* newValues = 0;
	glbGpuAllocator->_cudaMalloc((void**)&newKeys, (int)(numKeys * sizeof(int)));
	glbGpuAllocator->_cudaMalloc((void**)&newValues, (int)(numKeys * sizeof(int)));
	cudaMemcpy(newKeys, keys, (int)(numKeys * sizeof(int)), cudaMemcpyHostToDevice);
	cudaMemcpy(newValues, values, (int)(numKeys * sizeof(int)), cudaMemcpyHostToDevice);

	//Seeing if the hashTable needs resizing based on its comparison to the loadFactor
	if (tempSize + numKeys > totalSize * 0.9) {
		reshape(totalSize * 2);
	}

	//Copy-ing the new size(in case it changed, else it is the same one but we still copy it) to the Device
	int* newtempSize = 0;
	glbGpuAllocator->_cudaMalloc((void**)&newtempSize, sizeof(int));
	cudaMemcpy(newtempSize, &tempSize, sizeof(int), cudaMemcpyHostToDevice);

	//Calculate the number of blocks based on the number of keys and available threads
	int blocks, threads;
    cudaDeviceProp DeviceProp;
    cudaGetDeviceProperties(&DeviceProp, 0);
    threads = DeviceProp.maxThreadsPerBlock;
    if (numKeys % threads == 0) {
        blocks = numKeys / threads;
    }
    else {
        blocks = numKeys / threads + 1;
    }

	//Inserting the batch
	insertBatchKernel<<<blocks, threads>>>(hashTable, newKeys, newValues, numKeys, totalSize, newtempSize);
	//Ensuring that all processes are completed before the next thread continues executing the next operation
	cudaDeviceSynchronize();

	//Copy-ing the size after its change to the Host
	cudaMemcpy(&tempSize, newtempSize, sizeof(int), cudaMemcpyDeviceToHost);

	//Free-ing the variables used for insertBatch
	glbGpuAllocator->_cudaFree(newKeys);
	glbGpuAllocator->_cudaFree(newValues);
	glbGpuAllocator->_cudaFree(newtempSize);

	return true;
}

/**
 * Function getBatch
 * Gets a batch of key:value, using GPU
 */
int* GpuHashTable::getBatch(int* keys, int numKeys) {

	//Initializing the new keys and values, allocating memory for them, and copy-ing them from the Host to the Device
	int* newKeys;
	int* newValues;
	glbGpuAllocator->_cudaMalloc((void**)&newKeys, (int)(numKeys * sizeof(int)));
	glbGpuAllocator->_cudaMalloc((void**)&newValues, (int)(numKeys * sizeof(int)));
	cudaMemcpy(newKeys, keys, (int)(numKeys * sizeof(int)), cudaMemcpyHostToDevice);
	
	//Calculate the number of blocks based on the number of keys and available threads
	int blocks, threads;
    cudaDeviceProp DeviceProp;
    cudaGetDeviceProperties(&DeviceProp, 0);
    threads = DeviceProp.maxThreadsPerBlock;
    if (numKeys % threads == 0) {
        blocks = numKeys / threads;
    }
    else {
        blocks = numKeys / threads + 1;
    }

	//Returning the batch
	getBatchKernel<<<blocks, threads>>>(hashTable, newKeys, newValues, numKeys, totalSize);
	//Ensuring that all processes are completed before the next thread continues executing the next operation
	cudaDeviceSynchronize();

	//Copy-ing the new values to the host and storing them so we can return them 
	int* result = (int*)malloc((int)(numKeys * sizeof(int)));
	cudaMemcpy(result, newValues, (int)(numKeys * sizeof(int)), cudaMemcpyDeviceToHost);

	//Free-ing the variables used for getBatch
	glbGpuAllocator->_cudaFree(newKeys);
	glbGpuAllocator->_cudaFree(newValues);

	return result;
}
