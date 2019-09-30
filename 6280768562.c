/* Compile with -std=c99 */
#include<stdio.h>

void find_two_largest(int a[], int n, int *largest, int *second_largest);

int
main(int argc, char* argv[])
{
    int length, max, second_max;
    printf("Enter length of array: ");
    if (scanf("%d", &length) != 1 || length < 2) {
        return -1;
    }
    int a[length];
    for (int i = 0; i < length; i++) {
        printf("Enter the no.%d element: ", i + 1);
        scanf("%d", &a[i]);
    }
    find_two_largest(a, length, &max, &second_max);
    printf("largest=%d, second largest=%d\n", max, second_max);
    return 0;
}


void
find_two_largest(int a[], int n, int *largest, int *second_largest)
{
    int max = a[0];
    int second_max = a[0];
    for (int i = 1; i < n; i++) {
        if (max < a[i]) {
            second_max = max;
            max = a[i];
        }
    }
    *largest = max;
    *second_largest = second_max;
}
