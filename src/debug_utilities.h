#include <stdio.h>

#define DEBUG_MODE 1

#if DEBUG_MODE

    #define dbug_n(function_name) \
        fprintf(stderr, \
        "-------------------------------------------------------------------\nFunction:  %s\n\n"\
        , function_name)

    #define dbug_e(function_name) \
        fprintf(stderr, \
        "\nEnd of function:  %s\n-------------------------------------------------------------------\n"\
        , function_name)

    #define dbug_formatted_print(format, parameter) \
        fprintf(stderr, format, parameter)

    #define dbug_print(parameter) \
        fprintf(stderr, parameter)

#else

    #define dbug_n(function_name) ;

    #define dbug_e(function_name) ;

    #define dbug_formatted_print(format, parameter) ;

    #define dbug_print(parameter) ;

#endif