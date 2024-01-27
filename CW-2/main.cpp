/*
------------------DR VASILIOS KELEFOURAS-----------------------------------------------------
------------------COMP1001 ------------------------------------------------------------------
------------------COMPUTER SYSTEMS MODULE-------------------------------------------------
------------------UNIVERSITY OF PLYMOUTH, SCHOOL OF ENGINEERING, COMPUTING AND MATHEMATICS---
*/


#include <stdio.h> // for input/output functions
#include <time.h> // time related predefined library
#include <pmmintrin.h> // SSE and AVX intrinsics
#include <process.h> // for system() function
#include <chrono> //for high_resolution_clock
#include <iostream> // for input/output functions
#include <immintrin.h> // AVX intrinsics
#include <omp.h> //openmp library



#define M 1024*512 // size of arrays for routine1
#define ARITHMETIC_OPERATIONS1 3*M // number of arithmetic operations in routine1
#define TIMES1 1 // number of times routine1 is executed

#define N 8192 // size of arrays for routine2
#define ARITHMETIC_OPERATIONS2 4*N*N // number of arithmetic operations in routine2
#define TIMES2 1 // number of times routine2 is executed

#define EPSILON 0.0001 // tolerance of which two floats are considered equal


//function declarations
void initialize(); // init function intializes the arrays

void routine1(float alpha, float beta); // calls routine1 with alpha and beta
void routine2(float alpha, float beta); // calls routine2 with alpha and beta

void routine1_vec(float alpha, float beta); // calls routine1_vec with alpha and beta
void routine2_vec(float alpha, float beta); // calls routine2_vec with alpha and beta

unsigned short int equal(float a, float b); // function to check equality within tolerance
/*
    Routine1: y[i] = (alpha * y[i]) + (beta * z[i]);
    Routine2: w[i] = w[i] = (w[i] - beta) + (alpha * A[i][j] * x[j]);

    declspec(align(64)) is used to align the arrays to 64-byte boundaries

    M is the size of the arrays for routine1
    N is the size of the arrays for routine2

    [x] TODO: Make routine2_vec
    [x] TODO: TEST IF VECTORISED CALCULATIONS = NON-OPTIMISED VERSION WITH COMPARE FUNCTION
    [x] RESULTS DO NOT MATCH. it must be the compare function because routine1 must be the same

    git still tracks my .vs folder even though i have .vs/ in my .gitignore file
*/

__declspec(align(64)) float  y[M], z[M]; // declare arrays as 64-byte aligned
__declspec(align(64)) float A[N][N], x[N], w[N]; // declare arrays as 64-byte aligned

__declspec(align(64)) float y_copy[M]; // declare array that holds cloned y array for testing
__declspec(align(64)) float w_copy[N]; // declare array that holds cloned w array for testing

int main() {

    float alpha = 0.023f; // float value of 0.023 for beta alpha variable
    float beta = 0.045f; // float value of 0.045 for float beta variable

    double run_time; // double-type variable to store the execution time, for testing 
    double start_time; // double-type variable to store the start time, for testing
    
    unsigned int t; // unsigned int-type variable to store the number of times the routines are executed    

    initialize(); // initialise the arrays




    /*

    EXAMPLE FROM LAB-SESSION:
    
    unsigned short int ConstAdd_AVX() {

	    __m256  ymm1, ymm2, ymm3;
	    int i;

	    ymm1 = _mm256_set_ps(2.1234f, 2.1234f, 2.1234f, 2.1234f, 2.1234f, 2.1234f, 2.1234f, 2.1234f); //set num1 values
	    for (i = 0; i < M; i += 8) { //IMPORTANT: M MUST BE A MULTIPLE OF 8, OTHERWISE IT DOES NOT WORK
		    ymm2 = _mm256_loadu_ps(&V2[i]); //load 8 elements of X2[]
		    ymm3 = _mm256_add_ps(ymm1, ymm2); //num3 = num1 + num2
		    _mm256_storeu_ps(&V1[i], ymm3); //store num3 to Y[i]. num3 has 8 FP values which they are stored into Y[i], Y[i+1], Y[i+2], Y[i+3], .. Y[i+7]
	    }


	    return 2;
    }
    
    */



    printf("\n-----------------NON-OPTIMISED------------------------------\n");

    printf("\nRoutine1:");
    start_time = omp_get_wtime(); //start timer

    for (t = 0; t < TIMES1; t++) // for loop to execute routine1 TIMES1 times
        routine1(alpha, beta); // init with alpha and beta
    
    run_time = omp_get_wtime() - start_time; //end timer
    printf("\n Time elapsed is %f secs \n %e FLOPs achieved\n", run_time, (double)(ARITHMETIC_OPERATIONS1) / ((double)run_time / TIMES1)); // print testing
    std::copy(y, y + M, y_copy); // copy routine1 to test comparison with vectorised version, y+M to check for last element
    
    initialize(); // reinitialise the arrays 

    printf("\nRoutine2:");
    start_time = omp_get_wtime(); //start timer

    for (t = 0; t < TIMES2; t++)
        routine2(alpha, beta);
    
    
    run_time = omp_get_wtime() - start_time; //end timer
    printf("\n Time elapsed is %f secs \n %e FLOPs achieved\n", run_time, (double)(ARITHMETIC_OPERATIONS2) / ((double)run_time / TIMES2)); // print testing
    std::copy(w, w + N, w_copy); // copy routine2 to test comparison with vectorised version, w+N to check for last element

    initialize(); // reinitialise arrays 

    printf("\n-----------------VECTORISED------------------------------\n");

    // [x] make copies before running vectorised versions


    printf("\nRoutine1_vec:");
    start_time = omp_get_wtime(); //start timer

    for (t = 0; t < TIMES1; t++) // for loop to execute routine1 TIMES1 times
        routine1_vec(alpha, beta); // init with alpha and beta

    // y now becomes what the vectorised version of routine1 has calculated

    run_time = omp_get_wtime() - start_time; //end timer
    printf("\n Time elapsed is %f secs \n %e FLOPs achieved\n", run_time, (double)(ARITHMETIC_OPERATIONS1) / ((double)run_time / TIMES1)); // print testing

    initialize(); // reinitialise the arrays

    printf("\nRoutine2_vec:");
    start_time = omp_get_wtime(); //start timer

    for (t = 0; t < TIMES2; t++)
        routine2_vec(alpha, beta);

    // w now becomes what the vectorised version of routine2 has calculated

    run_time = omp_get_wtime() - start_time; //end timer
    printf("\n Time elapsed is %f secs \n %e FLOPs achieved\n", run_time, (double)(ARITHMETIC_OPERATIONS2) / ((double)run_time / TIMES2)); // print testing

    printf("\n-----------------TESTING------------------------------\n\n");

    bool arraysAreEqual; // boolean to check if arrays are equal

    arraysAreEqual = false;
    for (int i = 0; i < M; i++) {
        if (equal(y_copy[i], y[i])) { // Use the equal function to compare elements
            arraysAreEqual = true;
            break;
        }
    }

    if (arraysAreEqual) {
        printf("Routine1_vec: Results match.\n");
    }
    else {
        printf("Routine1_vec: Results do not match!\n");
    }

    arraysAreEqual = false;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
			if (equal(w_copy[i], w[i])) { // Use the equal function to compare elements
            	arraysAreEqual = true;
            	break;
            }
		}
    }

    if (arraysAreEqual) {
		printf("Routine2_vec: Results match.\n");
	}
    else {
        printf("Routine2_vec: Results do not match!\n");
    }

    return 0; // return 0 to indicate that program has finished successfully
}

void initialize() {

    unsigned int i, j;

    //initialize routine2 arrays with random values
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++) {
            A[i][j] = (i % 99) + (j % 14) + 0.013f;
        }

    //initialize routine1 arrays with random values
    for (i = 0; i < N; i++) {
        x[i] = (i % 19) - 0.01f;
        w[i] = (i % 5) - 0.002f;
    }

    //initialize routine1 arrays
    for (i = 0; i < M; i++) {
        z[i] = (i % 9) - 0.08f;
        y[i] = (i % 19) + 0.07f;
    }


}

void routine1(float alpha, float beta) { // routine1: y[i] = alpha * y[i] + beta * z[i];

    unsigned int i; // loop counter

    for (i = 0; i < M; i++)
        y[i] = (alpha * y[i]) + (beta * z[i]); // for each iteration of the array, i = (alpha * y[i]) + (beta * z[i])
}

// AVX implementation of routine1

void routine1_vec(float alpha, float beta) {

    unsigned int i; // loop counter

    // Create AVX vectors for alpha and beta
    __m256 alpha_vec = _mm256_set1_ps(alpha); // set1_ps sets all elements of alpha to alpha_vec, which holds 8 elements
    __m256 beta_vec = _mm256_set1_ps(beta); // set1_ps sets all elements of beta to beta_vec, which holds 8 elements
    // essentially there's 8 seperate float beta = 0.045f; iterations
    // beta_vec = |0.045|0.045|0.045|0.045|0.045|0.045|0.045|0.045|

    // process 8 elements at a time for each iteration, until i reaches M
    for (i = 0; i < M; i += 8) {

        // & means the address of the variable, so &y[i] means the address of the y[i] variable
        __m256 y_vec = _mm256_load_ps(&y[i]); // load 8 seperate iterated elements from y into AVX register 
        __m256 z_vec = _mm256_load_ps(&z[i]); // load 8 seperate iterated elements from z into AVX register

        // Perform the vectorized operations
        __m256 result_vec = _mm256_add_ps(_mm256_mul_ps(alpha_vec, y_vec),_mm256_mul_ps(beta_vec, z_vec));
        // result_vec = (alpha * y_vec) + (beta * z_vec)

    // Store the results back into the y array
        _mm256_store_ps(&y[i], result_vec);
    }

    // IS THIS NECCESSARY? [x]
    // No because M is a multiple of 8, so the loop will always be executed?

}

// from lab-session:
unsigned short int equal(float a, float b) {
    float temp = a - b;
    //printf("\n %f  %f", a, b);
    if ((fabs(temp) / fabs(b)) < EPSILON)
        return 0; //success
    else
        return 1; //wrong result
}


void routine2(float alpha, float beta) {

    unsigned int i, j;


    for (i = 0; i < N; i++) // 2d array iteration?
        for (j = 0; j < N; j++)
            w[i] = (w[i] - beta) + (alpha * A[i][j] * x[j]);

}

//void routine2_vec(float alpha, float beta) {
//    unsigned int i, j;
//
//    // Broadcast alpha and beta to all elements of 256-bit vectors
//    __m256 alpha_vec = _mm256_set1_ps(alpha);
//    __m256 beta_vec = _mm256_set1_ps(beta);
//
//    for (i = 0; i < N; i++) {
//        // Start with w[i] and broadcast it to all elements of a vector
//        __m256 w_vec = _mm256_set1_ps(w[i]);
//
//        // Subtract beta from w[i]
//        w_vec = _mm256_sub_ps(w_vec, beta_vec);
//
//        // Initialize a vector to accumulate the results
//        __m256 sum_vec = _mm256_setzero_ps();
//
//        for (j = 0; j < N; j += 8) {
//            // Load 8 consecutive values from the matrix row and vector x
//            __m256 a_vec = _mm256_load_ps(&A[i][j]);
//            __m256 x_vec = _mm256_load_ps(&x[j]);
//
//            // Perform the multiplication and accumulate the results
//            sum_vec = _mm256_add_ps(sum_vec, _mm256_mul_ps(a_vec, x_vec));
//        }
//
//        // Sum all elements of sum_vec into a single value
//        sum_vec = _mm256_hadd_ps(sum_vec, sum_vec); // Horizontally add pairs of elements
//        sum_vec = _mm256_hadd_ps(sum_vec, sum_vec); // Repeat to sum all elements
//
//        // Extract the final result from the sum_vec
//        float temp[8]; // Temporary array to store the results
//        _mm256_store_ps(temp, sum_vec);
//
//        // The horizontal add above has added pairs, and the result we need is in the first and fifth element
//        w[i] = temp[0] + temp[4] * alpha - beta; // Add the accumulated sum to w[i] (accounting for the initial subtraction)
//    }
//}

void routine2_vec(float alpha, float beta) {

    unsigned int i = 0, j = 0; // init loop counters with 0 for i and j, 

    /*

        set1_ps sets all elements of alpha and beta to a vector
        which each hold 8 elements. It could also be said to broadcast the values
        to all elements of the 256-bit vectors, you essentially replicate a
        single float value across all elements of each 256 - bit vector

    */

    __m256 alpha_vec = _mm256_set1_ps(alpha);
    __m256 beta_vec = _mm256_set1_ps(beta);

    /*
        
        init before asigning values to vec_A, vec_B, vec_C,
        these values are used for the calculations, so want to
        be initialised before the loop?

    */

    for (i = 0; i < N; i++) {
        /*
            
            load in values for the 3 different looping variables, a_vec has a 2d-loop thus requires 'i' and 'j'
        
        */

        __m256 a_vec = _mm256_set1_ps(A[i][j]);
        __m256 x_vec = _mm256_set1_ps(x[j]);
        __m256 w_vec = _mm256_set1_ps(w[j]);

        // outer loop for rows of matrix A and vector. 
        __m256 sum_vec = _mm256_setzero_ps(); // Initialise the accumulator vector as zero

        for (j = 0; j < N; j += 8) {

            __m256 a_vec = _mm256_load_ps(&A[i][j]); // Load elements from A
            __m256 w_vec = _mm256_load_ps(&w[i]); // Load elements from w
            __m256 x_vec = _mm256_load_ps(&x[j]); // Load elements from x


            beta_vec = _mm256_load_ps(&beta);

            __m256 vec_A = _mm256_mul_ps(alpha_vec, a_vec); // Compute (alpha*A[i][j])
            __m256 vec_B = _mm256_mul_ps(vec_A, x_vec); // Compute (vec_A) * x[j]
            __m256 vec_C = _mm256_sub_ps(w_vec, beta_vec); // Compute (w[i] - b)

            // Horizontal add to sum up elements of sum_vec
            sum_vec = _mm256_add_ps(vec_B, vec_C);
            //printf("sum_vec: %f\n", sum_vec); Can't print mm256 types

            


            // (w[i] - beta) + ((alpha * A[i][j]) * x[j]);

            /*
                Order of Operations:

                1. alpha*A[i][j] - A
                2. A *x[j] - B
                3. (w[i] - beta) - C
                4. B+C - D
            */
        }

        // Store calculations that're complete for 8 single-precision, floating-point values
        _mm256_store_ps(&w[i], sum_vec);
    }
}




/*
     ** AVX intrinsic Functions: **

       __m256: means a 256-bit variable that can hold eight 32-bit single-precision floating-point values.
       _mm256_load_ps: loads eight 32-bit single-precision floating-point values.
       _mm256_store_ps: stores eight 32-bit single-precision floating-point values into memory.
       _mm256_add_ps: adds eight 32-bit single-precision floating-point values.
       _mm256_sub_ps: subtracts eight 32-bit single-precision floating-point values.
       _mm256_mul_ps: multiplies eight 32-bit single-precision floating-point values
       _mm256_setzero_ps: initializes all elements of this 256-bit vector to zero.
       _mm256_set1_ps: initializes all elements of this 256-bit vector with the same single-precision floating-point value.
       _mm256_fmad_ps: performs a fused multiply-add operation on eight 32-bit single-precision floating-point values.
       _mm256_hadd_ps: horizontally adds adjacent pairs of 32-bit single-precision floating-point values.

   /*



/*
    Routine2:  2d i + j loop->w[i] = (w[i] - beta) + (alpha * A[i][j] * x[j]);

    // decleration of arrays and align as 64-bit
    __declspec(align(64)) float  y[M], z[M];
    __declspec(align(64)) float A[N][N], x[N], w[N];



*/




















































