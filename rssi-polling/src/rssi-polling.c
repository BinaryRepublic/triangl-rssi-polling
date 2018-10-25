#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#define __USE_XOPEN
#define _GNU_SOURCE
#include <time.h>
#include "json.c"
#include <curl/curl.h>


char *read_mac_address(char *path);
char **split_trim_str(char *str, char delim);
void free_split(char **split);
char *station_to_json(char **station_data);
time_t get_timestamp(char * str);
void post(char *json);
char *evaluate_csv(char *path);

int last_upload = 0;
char *mac_addr = NULL;

int main(int argc, char const *argv[])
{
    char *path_mac_file = "/triangl-package-updater/my_mac";
    char *path_csv = "/triangl-package-updater/airodump-01.csv";

    mac_addr = read_mac_address(path_mac_file);
    while (1)
    {
        char *json_output = evaluate_csv(path_csv);
        if (json_output == NULL)
            sleep(2);
        else
        {
            post(json_output);
            free(json_output);
        }
    }
    free(mac_addr);
}


char *read_mac_address(char *path)
{
    size_t len = 0;
    FILE *mac_fp = fopen(path, "r");
    char *mac_addr = NULL;

    if (mac_fp == NULL)
    {
        fprintf(stderr, "Failed to open the File containg the devices mac address!\n");
        exit(1);
    }
    getline(&mac_addr, &len, mac_fp);
    fclose(mac_fp);
    len = strlen(mac_addr);
    mac_addr[len-1] = mac_addr[len-1] == '\n' ? '\0' : mac_addr[len-1];
    return(mac_addr);
}

char *evaluate_csv(char *path)
{
    char * line = (char *)malloc(240);
    size_t len = 240;
    int is_station = 0;
    int max_timestamp = last_upload;
    char *json_arr = create_json_array(65536);
    FILE *csv = fopen(path, "r");

    if (csv != NULL)
    {
    while(getline(&line, &len, csv) > 0)
    {
        if (-44 == strcmp("Station MAC", line))
            is_station = 1;
        else if (is_station && strlen(line) > 62)
        {
            char **station_data = split_trim_str(line, ',');
            int last_seen = get_timestamp(station_data[2]);
            max_timestamp = last_seen > max_timestamp ? last_seen : max_timestamp;            
            if(last_seen > last_upload)
                json_array_add_json(json_arr, station_to_json(station_data));
            free_split(station_data);
        }
    }
    }
    last_upload = max_timestamp;
    free(line);
    fclose(csv);
    if (strlen(json_arr) == 2) //json empty
    {
        free(json_arr);
        return (NULL);
    }
    return(json_arr);
}

time_t get_timestamp(char * str)
{
    struct tm tm;
    if (strptime(str, "%Y-%m-%d %H:%M:%S", &tm) != NULL)
        return(mktime(&tm));
    else
        return 0;
}

char *station_to_json(char **station_data)
{
    char *json = create_json(240);

    json_add_str(json, "deviceId", station_data[0]);
    json_add_str(json,"timestampString",station_data[2]);
    //json_add_int(json, "signalStrength", atoi(station_data[3]));
    json_add_charint(json,"signalStrength", station_data[3]);
    json_add_str(json, "routerId", mac_addr);
    json_add_str(json, "associatedAP", station_data[5]);
    return(json);
}

char *trim (char *s)
{
    //https://stackoverflow.com/questions/46628126/removing-a-space-newline-and-tabs-with-string-trimming-in-c
    char *r = (char *)malloc(strlen(s));
    char *p = r;                            /* pointer to result         */
    char *sc = s;
    *r = 0;                                 /* initialize as empty str   */
    if (!s) return NULL;                    /* validaate source str      */
    if (!*s) return r;                      /* empty str - nothing to do */
    while (isspace (*s))  s++;              /* skip leading whitespace   */
    while (*s) *p++ = *s++;                 /* fill r with s to end      */
    *p = 0;                                 /* nul-terminate r           */
    while (p > r && isspace (*--p)) *p = 0; /* overwrite spaces from end */
    while (p > r && !isalnum (*--p)) {      /* continue until 1st alnum  */
        if (isspace (*p)) {                 /* if spaces found           */
            char *rp = p, *wp = p;          /* set read & write pointers */
            while (*rp++) *wp++ = *rp;      /* shuffle end chars forward */
            *wp = 0;                        /* nul-terminate at new end  */
        }
    }
    return r;
}

char **split_trim_str(char *str, char delim)
{
    int len = strlen(str);
    int segments = 1; //first comman occurs at the end of the first segment

    for(int i=0;i<len;i++)
    {
        if (str[i] == delim)
        {
            str[i] = '\0';
            segments += 1;
        }
    }
    char **splited_str = (char **)malloc((segments + 1) * sizeof(char *));
    char *str_cpy = str; 
    int seg = 0;
    for (int i=0;i<len;i++)
    {
        if (str_cpy[i] == '\0' && str != (str + i))
        {
            // trim enables the function to trim and split simultaneously,
            // to only split replace trim with malloc
            splited_str[seg++] = trim(str); 
            str = i+1 >= len? str : str_cpy+i+1;
        }
    }
    splited_str[seg] = NULL;
    return(splited_str);
}

void free_split(char **split)
{
    int i = 0;
    while(split[i] != NULL)
        free(split[i++]);
    free(split);
}

void post(char *json)
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *header = NULL;
    header = curl_slist_append(header, "Content-Type: application/json");
    curl_slist_append(header, "charsets: utf-8");

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.triangl.io/tracking-ingestion-service/tracking/multiple");

        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcrp/0.1");
        fprintf(stderr, "%s\n", json);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "%d - curl_easy_perform() failed, Error message : %s\n", last_upload,
                    curl_easy_strerror(res));
        else
            fprintf(stderr, "%d - JSON send sucessfully\n", last_upload);

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}