#include <stdio.h>

typedef enum
{
    STAT_OK = 0,

    /* Общие ошибки */
    INVALID_INPUT,
    INPUT_OVERFLOW,
    INVALID_ARGUMENT,
    INVALID_FLAG,
    MATCH_ARGS,

    /* Арифметические ошибки */
    ZERO_DIV,
    FUNC_OVERFLOW,
    NEGATIVE_NUMBER,
    NEG_SIDES,
    INVALID_BASE,
    INVALID_NUMBER,
    NOT_PRIME,

    /* Численные методы и вещественные вычисления */
    INVALID_EPSILON,
    INVALID_INTERVAL,
    NO_ROOT_INTERVAL,
    NO_CONVERGENCE,
    INVALID_FUNCTION,

    /* Ошибки памяти */
    MEMORY_ERROR,
    REALLOC_ERROR,

    /* Ошибки файлов */
    FILENAME_ERROR,
    OPENFILE_ERROR,
    READFILE_ERROR,
    WRITEFILE_ERROR,
    CLOSEFILE_ERROR,

    /* Строки и преобразования */
    EMPTY_STRING,
    INVALID_CHARACTER,
    INVALID_NUMBER_FORMAT,
    STRING_OVERFLOW,
    CONVERSION_ERROR,

    /* Массивы */
    EMPTY_ARRAY,
    ARRAY_BOUNDS_ERROR,
    INVALID_ARRAY_SIZE,

    /* Переполнение результата */
    FACTORIAL_OVERFLOW,
    FACDOUB_OVERFLOW,
    POWER_OVERFLOW,
    RESULT_OVERFLOW,

    UNKNOWN_ERROR

} ErrorsDef;

void errors(ErrorsDef e)
{
    switch (e)
    {
        case STAT_OK:
            return;

        case INVALID_INPUT:
            puts("Error! Invalid input!");
            break;

        case INPUT_OVERFLOW:
            puts("Error! Input overflow!");
            break;

        case INVALID_ARGUMENT:
            puts("Error! Invalid argument!");
            break;

        case INVALID_FLAG:
            puts("Error! Invalid flag!");
            break;

        case MATCH_ARGS:
            puts("Error! Arguments do not match the flag!");
            break;

        case ZERO_DIV:
            puts("Error! Division by zero!");
            break;

        case FUNC_OVERFLOW:
            puts("Error! Function overflow!");
            break;

        case NEGATIVE_NUMBER:
            puts("Error! Negative number!");
            break;

        case NEG_SIDES:
            puts("Error! Side values cannot be negative!");
            break;

        case INVALID_BASE:
            puts("Error! Invalid base!");
            break;

        case INVALID_NUMBER:
            puts("Error! Invalid number!");
            break;

        case NOT_PRIME:
            puts("Error! Number is not prime!");
            break;

        case INVALID_EPSILON:
            puts("Error! Invalid epsilon!");
            break;

        case INVALID_INTERVAL:
            puts("Error! Invalid interval!");
            break;

        case NO_ROOT_INTERVAL:
            puts("Error! Root is not bracketed in the interval!");
            break;

        case NO_CONVERGENCE:
            puts("Error! Method did not converge!");
            break;

        case INVALID_FUNCTION:
            puts("Error! Invalid function!");
            break;

        case MEMORY_ERROR:
            puts("Error! Memory allocation failed!");
            break;

        case REALLOC_ERROR:
            puts("Error! Memory reallocation failed!");
            break;

        case FILENAME_ERROR:
            puts("Error! Invalid filename!");
            break;

        case OPENFILE_ERROR:
            puts("Error! Could not open file!");
            break;

        case READFILE_ERROR:
            puts("Error! Could not read file!");
            break;

        case WRITEFILE_ERROR:
            puts("Error! Could not write to file!");
            break;

        case CLOSEFILE_ERROR:
            puts("Error! Could not close file!");
            break;

        case EMPTY_STRING:
            puts("Error! Empty string!");
            break;

        case INVALID_CHARACTER:
            puts("Error! Invalid character!");
            break;

        case INVALID_NUMBER_FORMAT:
            puts("Error! Invalid number format!");
            break;

        case STRING_OVERFLOW:
            puts("Error! String overflow!");
            break;

        case CONVERSION_ERROR:
            puts("Error! Conversion failed!");
            break;

        case EMPTY_ARRAY:
            puts("Error! Array is empty!");
            break;

        case ARRAY_BOUNDS_ERROR:
            puts("Error! Array index out of bounds!");
            break;

        case INVALID_ARRAY_SIZE:
            puts("Error! Invalid array size!");
            break;

        case FACTORIAL_OVERFLOW:
            puts("Error! Factorial overflow!");
            break;

        case POWER_OVERFLOW:
            puts("Error! Power overflow!");
            break;

        case RESULT_OVERFLOW:
            puts("Error! Result overflow!");
            break;

        case UNKNOWN_ERROR:
        default:
            puts("Error! Unknown error!");
            break;
    }
}