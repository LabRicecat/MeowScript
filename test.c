#include <stdio.h> // including of needed functions (prinf,scanf)

int main() { // Begin of main()
    int num1 = 0, num2 = 0, res = 0; // Init all variables

    printf("Enter number 1: ");
    scanf("%d",&num1); // get first number
    printf("Enter number 2: ");
    scanf("%d",&num2); // get second number
    res = num1 + num2; // do calculations
    printf("The final result is: %d\n",res); // print result

    return 0; // tell the OS that everything went fine
}