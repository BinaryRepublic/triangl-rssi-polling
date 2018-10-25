#include <string.h>
#include <stdlib.h> 

char    *itoa(int nb);
void strcat_quoted(char *dest, char *src);
void prepare_object(char *json);

char *create_json(int max_length)
{
    char *json = (char *)malloc(max_length);
    strcpy(json, "{}\0");
    return(json);
}

char *create_json_array(int max_length)
{
    char *json_arr = (char *)malloc(max_length);
    strcpy(json_arr, "[]\0");
    return (json_arr);
}

char *json_array_add_json(char *json_arr, char *json)
{
    prepare_object(json_arr);
    strcat(json_arr, json);
    free(json);
    strcat(json_arr, "\n]");
}

char *json_add_int(char *json, char *key, int value)
{
    prepare_object(json);
    strcat_quoted(json, key);
    strcat(json, ":");
    char *str_value = itoa(value);
    strcat(json, str_value);
    free(str_value);
    strcat(json, "\n}");
}

/*
* This function adds an string containing an integer as integer to the json
*/
char *json_add_charint(char *json, char *key, char *value)
{
    prepare_object(json);
    strcat_quoted(json, key);
    strcat(json, ":");
    strcat(json, value);
    strcat(json, "\n}");

}

char *json_add_str(char *json, char *key, char *value)
{
    prepare_object(json);
    strcat_quoted(json, key);
    strcat(json, ":");
    strcat_quoted(json, value);
    strcat(json, "\n}");
}

void prepare_object(char *json)
{
    char *json_end = json + strlen(json) - 2;
    if (json != json_end)
        strcpy(json_end,",\n\0"); //override end of the json
    else
        strcpy(json+1, "\n\0");
}

void strcat_quoted(char *dest, char *src)
{
    strcat(dest, "\"");
    strcat(dest, src); 
    strcat(dest, "\"");
}

char *itoa(int nb)
{
    char *str;
    int nb_cpy;
    int sign_flag= 0;
    int i = sign_flag?0:1;

    nb *= nb<0?(sign_flag=-1):1;
    str = malloc(sign_flag?11:12);
    nb_cpy = nb;
    while (nb_cpy || !i)
    {
        nb_cpy /= 10;
        str[i++] = '\0';
    }
    str[i] = '\0';
    while (i-- > 0)
    {
        if (sign_flag && i==0)
            str[i] = '-';
        else
            str[i] = '0' + nb % 10;
        nb /= 10;
    }
    return (str);
}