/**
 * @file sort_iterative.c
 * @brief Example program to demonstrate bitonic sorting and file operations in C.
 */

#include <stdio.h>
#include <stdlib.h>

/**
 * Compares two integers for ascending order.
 * 
 * @param a Pointer to the first integer to compare.
 * @param b Pointer to the second integer to compare.
 * @return An integer less than, equal to, or greater than zero if the first argument
 * is considered to be respectively less than, equal to, or greater than the second.
 */
int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

/**
 * Compares two integers for descending order.
 * 
 * @param a Pointer to the first integer to compare.
 * @param b Pointer to the second integer to compare.
 * @return An integer less than, equal to, or greater than zero if the first argument
 * is considered to be respectively greater than, equal to, or less than the second.
 */
int compare_desc(const void *a, const void *b) {
    return (*(int *)b - *(int *)a);
}

/**
 * Swaps two integers if the first is greater than the second.
 * 
 * @param a Pointer to the first integer.
 * @param b Pointer to the second integer.
 */
void CAPS(int *a, int *b) {
    if (*a > *b) {
        int temp = *a;
        *a = *b;
        *b = temp;
    }
}

/**
 * Performs the bitonic merge for the iterative bitonic sort algorithm.
 * 
 * @param val Pointer to the array of integers to sort.
 * @param low Starting index for the merge process.
 * @param count Number of elements to include in the merge.
 * @param dir Direction of sorting: 1 for ascending, 0 for descending.
 */
void bitonicMergeIterative(int *val, int low, int count, int dir) {
    for (int k = 2; k <= count; k = 2 * k) {
        for (int i = low; i < low + count; i += k) {
            int mid = k / 2;
            for (int j = i; j < i + mid; j++) {
                if (dir == (val[j] > val[j + mid])) {
                    int temp = val[j];
                    val[j] = val[j + mid];
                    val[j + mid] = temp;
                }
            }
        }
    }
}

/**
 * Sorts an array of integers using the iterative bitonic sorting algorithm.
 * 
 * @param val Pointer to the array of integers to sort.
 * @param N The number of elements in the array.
 */
void bitonicSortIterative(int *val, int N) {
    for (int k = 2; k <= N; k = 2 * k) {
        for (int i = 0; i < N; i += k) {
            int dir = ((i / (k / 2)) % 2) == 0;
            bitonicMergeIterative(val, i, k, dir);
        }
    }
}

/**
 * Main function. Reads integers from a binary file, performs sorting, and prints the results.
 * 
 * @param argc The number of command-line arguments.
 * @param argv The command-line arguments, including the program name and the file path.
 * @return EXIT_SUCCESS on successful execution, or EXIT_FAILURE on error.
 */
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    /* Read file */

    FILE *filePtr;
    int numberOfIntegers, i;
    int *val;

    // Open the file in binary read mode
    filePtr = fopen(argv[1], "rb");
    if (filePtr == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // Read the first integer to find out the number of integers in the file
    if (fread(&numberOfIntegers, sizeof(int), 1, filePtr) != 1) {
        perror("Error reading number of integers");
        fclose(filePtr);
        return EXIT_FAILURE;
    }

    // Dynamically allocate memory for the integers (excluding the first one)
    val = (int *)malloc(numberOfIntegers * sizeof(int));
    if (val == NULL) {
        perror("Memory allocation failed");
        fclose(filePtr);
        return EXIT_FAILURE;
    }

    // Read the integers into the array
    for (i = 0; i < numberOfIntegers; i++) {
        if (fread(&val[i], sizeof(int), 1, filePtr) != 1) {
            perror("Error reading an integer from the file");
            free(val); // Free allocated memory before exiting
            fclose(filePtr);
            return EXIT_FAILURE;
        }
    }

    // Close the file
    fclose(filePtr);
    

    /* Standard merge sorting */
    
    // Calculate the midpoint of the array to divide it into two halves
    int midPoint = numberOfIntegers / 2;

    // Sort the first half
    qsort(val, midPoint, sizeof(int), compare);

    // Sort the second half
    qsort(val + midPoint, numberOfIntegers - midPoint, sizeof(int), compare);

    printf("\nBefore sorting: \n");
    // Print the integers from the array
    for (i = 0; i < numberOfIntegers; i++) {
        if(i == numberOfIntegers/2)
        {
            printf("\n");
        }
        printf("%d\n", val[i]);
    }
    
    printf("\nAfter Standard merge sorting:\n");
    
    int N = numberOfIntegers;
    for (int m = 0; m < N/2; m++)
        for (int n = 0; (m + n) < N/2; n++)
            CAPS(&val[m+n], &val[N/2+n]);

    for (i = 0; i < numberOfIntegers; i++) {
        if(i == numberOfIntegers/2)
        {
            printf("\n");
        }
        printf("%d\n", val[i]);
    }

    /* After Standard merge sorting */
    for (i = 0; i < N - 1; i++) {
        if (val[i] > val[i + 1]) {
            printf("Error in position %d between element %d and %d\n", i, val[i], val[i + 1]);
            break;
        }
    }
    if (i == (N - 1)) {
        printf("Everything is OK with Standard merge sorting!\n");
    }


    /* Bitonic sorting */

    // Sort the first half
    qsort(val, midPoint, sizeof(int), compare);

    // Sort the second half
    qsort(val + midPoint, numberOfIntegers - midPoint, sizeof(int), compare_desc);

    printf("\nBefore bitonic sorting: \n");
    for (i = 0; i < numberOfIntegers; i++) {
        printf("%d\n", val[i]);
    }

    // Apply bitonic sort
    bitonicSortIterative(val, numberOfIntegers);

    printf("\nAfter bitonic sorting:\n");
    for (i = 0; i < numberOfIntegers; i++) {
        printf("%d\n", val[i]);
    }

    /* After Bitonic sorting */
    for (i = 0; i < N - 1; i++) {
        if (val[i] > val[i + 1]) {
            printf("Error in position %d between element %d and %d\n", i, val[i], val[i + 1]);
            break;
        }
    }
    if (i == (N - 1)) {
        printf("Everything is OK with Bitonic sorting!\n");
    }

    // Free the dynamically allocated memory
    free(val);
    return EXIT_SUCCESS;
}