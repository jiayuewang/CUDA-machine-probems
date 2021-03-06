#include <wb.h>

#define wbCheck(stmt)                                                     \
  do {                                                                    \
    cudaError_t err = stmt;                                               \
    if (err != cudaSuccess) {                                             \
      wbLog(ERROR, "Failed to run stmt ", #stmt);                         \
      wbLog(ERROR, "Got CUDA error ...  ", cudaGetErrorString(err));      \
      return -1;                                                          \
    }                                                                     \
  } while (0)

// Compute C = A * B
__global__ void matrixMultiplyShared(float* M, float* N, float* P,
                                     int numARows, int numAColumns, int numBRows, int numBColumns,
                                     int numCRows, int numCColumns){

  const int TILE_WIDTH = 32;
  

  __shared__ float subTileM [TILE_WIDTH][TILE_WIDTH];
  __shared__ float subTileN [TILE_WIDTH][TILE_WIDTH];
  
  int bx = blockIdx.x;
 
  int tx = threadIdx.x;
  
  int ty = threadIdx.y;   
   int by = blockIdx.y;
 -

int main(int argc, char **argv) {
  wbArg_t args;
  
  
  
  float *hostA; // The A matrix
  float *hostB; // The B matrix
  float *hostC; // The output C matrix
  float *deviceA;
  float *deviceB;
  float *deviceC;
  
  
  int numARows;    // number of rows in the matrix A
  int numAColumns; // number of columns in the matrix A
  int numBRows;    // number of rows in the matrix B
  int numBColumns; // number of columns in the matrix B
  int numCRows;    // number of rows in the matrix C (you have to set this)
  int numCColumns; // number of columns in the matrix C (you have to set
                   // this)
  
  
  const int TILE_WIDTH = 32;
  
  args = wbArg_read(argc, argv);

  wbTime_start(Generic, "Importing data and creating memory on host");
  hostB = (float *)wbImport(wbArg_getInputFile(args, 1), &numBRows,
                            &numBColumns);
  
  
  hostA = (float *)wbImport(wbArg_getInputFile(args, 0), &numARows,
                            &numAColumns);
  
  //@@ Set numCRows and numCColumns
  numCColumns = numBColumns;numCRows = numARows;
  
  
  
  //@@ Allocate the hostC matrix
  hostC = (float *)malloc(numCRows * numCColumns * sizeof(float)); // allocate memory as for a 1 dim array
  wbTime_stop(Generic, "Importing data and creating memory on host");
  
  
  
wbLog(TRACE, "B ", numBRows, " x2 ", numBColumns);
  wbLog(TRACE, "A ", numARows, " x1 ", numAColumns);
  

  wbTime_start(GPU, "Allocating GPU memory.");
  //@@ Allocate GPU memory here
  wbCheck(cudaMalloc((void **) &deviceA, numARows * numAColumns * sizeof(float))); 
  wbCheck(cudaMalloc((void **) &deviceB, numBRows * numBColumns * sizeof(float)));
  wbCheck(cudaMalloc((void **) &deviceC, numCRows * numCColumns * sizeof(float)));

  wbTime_stop(GPU, "Allocating GPU memory.");

  wbTime_start(GPU, "Copying input memory to the GPU.");
  //@@ Copy memory to the GPU here
  wbCheck(cudaMemcpy(deviceA, hostA, numARows * numAColumns * sizeof(float), cudaMemcpyHostToDevice));
  wbCheck(cudaMemcpy(deviceB, hostB, numBRows * numBColumns * sizeof(float), cudaMemcpyHostToDevice));

  wbTime_stop(GPU, "Copying input memory to the GPU.");

  //@@ Initialize the grid and block dimensions here
  dim3 DimGrid(numCColumns/TILE_WIDTH,numCRows/TILE_WIDTH,1);
  if (numCColumns%TILE_WIDTH) DimGrid.x++;
  if (numCRows%TILE_WIDTH) DimGrid.y++;
  dim3 DimBlock(TILE_WIDTH,TILE_WIDTH,1);

  wbTime_start(Compute, "Performing CUDA computation");
  //@@ Launch the GPU Kernel here
  
  
  
  
  matrixMultiplyShared<<<DimGrid,DimBlock>>>(deviceA, deviceB, deviceC, numARows, numAColumns, numBRows, numBColumns, numCRows, numCColumns);
 
  cudaDeviceSynchronize();
  wbTime_stop(Compute, "Performing CUDA computation");

  wbTime_start(Copy, "Copying output memory to the CPU");
  //@@ Copy the GPU memory back to the CPU here
  wbCheck(cudaMemcpy(hostC, deviceC, numCRows * numCColumns * sizeof(float), cudaMemcpyDeviceToHost)); 

  wbTime_stop(Copy, "Copying output memory to the CPU");

  wbTime_start(GPU, "Freeing GPU Memory");
  //@@ Free the GPU memory here
  cudaFree(deviceA);
  cudaFree(deviceB);
  cudaFree(deviceC);
  
  wbTime_stop(GPU, "Freeing GPU Memory");

  wbSolution(args, hostC, numCRows, numCColumns);

  free(hostA);
  free(hostB);
  free(hostC);

  return 0;
}