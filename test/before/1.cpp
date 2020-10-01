i#include <thrust/host_vector.h> 
#include <thrust/device_vector.h> 
#include <thrust/transform.h>
#include <thrust/sequence.h>
#include <thrust/functional.h>
#include <thrust/gather.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/transform_iterator.h>
#include <iostream>
#include <cublas_v2.h>
#include <sys/time.h>
 

using namespace std;

double wtime()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}

struct transpose_index : public thrust::unary_function<int, int>{
	int m, n;
	__host__ __device__
	transpose_index(int _n, int _m) : n(_n), m(_m){}
	__host__ __device__
	int operator()(int linear_index){
		int i = linear_index / n;
		int j = linear_index % n;
		return m * j + i;
	}
	
};


void transpose(int m, int n, thrust::device_vector<float>&data, thrust::device_vector<float>&result){
	thrust::counting_iterator<int> indices(0);
	thrust::gather
		(thrust::make_transform_iterator(indices, transpose_index(n, m)), 
		thrust::make_transform_iterator(indices, transpose_index(n, m)) + m*n, 
		data.begin(), 
		result.begin());	
}

void print(int m, int n, thrust::host_vector<float>& data){
	for(int i = 0; i < m; i++){
		for(int j = 0; j < n; j++){
			cout << data[i*n + j] << " ";
		}
		cout << endl;
	}
}

int main(int argc, char* argv[]){
	int m = 3;
	if (argc > 1) 
		sscanf(argv[1], "%d", &m);
	int n = m;
    double t = wtime();
    thrust::host_vector<float> data_h(m*n);
	thrust::sequence(data_h.begin(), data_h.end(), 1, 1);
	thrust::device_vector<float> data_v = data_h;
    thrust::host_vector<float> thrust_h(m*n);
	thrust::device_vector<float> thrust_t(m*n);
	thrust::host_vector<float> cuBLAS(m * n);
	thrust::device_vector<float> transposed_cuBLAS(m * n);
    float* in = thrust::raw_pointer_cast(data_v.data());
	float* out = thrust::raw_pointer_cast(transposed_cuBLAS.data());
    t = wtime() - t;
    printf("Allocated memory time (N = %d) %.2f\n", n*m, t);
	
	cout << "Original array" << endl;
	if(m < 5)	print(m,n,data_h);
	
	cout << "Thrust" << endl;
    t = wtime();
	transpose(m, n, data_v, thrust_t);
	thrust_h = thrust_t;
	
    t = wtime() - t;
	if(m < 5)	print(m,n,thrust_h);    
    printf("Time (N = %d) %.6f\n", n*m, t);
	cout << "cuBLAS" << endl;
	t = wtime();
	float alpha = 1;
	float beta = 0;
    cublasHandle_t handle;
	cublasCreate(&handle);
	cublasSgeam(handle, CUBLAS_OP_T, CUBLAS_OP_T, m, n, &alpha, in, n, &beta, in, n, out, m);
	cublasDestroy(handle);
	cuBLAS = transposed_cuBLAS;
	
    t = wtime() - t;;    
	if(m < 5)	print(n, m, cuBLAS);
    printf("Time (N = %d) %.6f\n", n*m, t);
   	return 0;
}
