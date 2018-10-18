#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <curl/curl.h> //just for curl

double get_datetime(void);
char	*itoa(int nb);
void RemoveSpaces(char* source);
char *split_input(char *line);
double date_to_double(char *str);
void removeJSONEntry(void);
int post(void);
void trim(char *str);
void mainLogic(void);

double lastProcessedTimestamp = 0;
char *outputJSON;
int JSONcount = 0;

int DEBUG_TRUE = 3; // Set Debuglevels
    // 0 : No messages
    // 1 : Only success and critical error messages
    // 2 : important iterative messages
    // 3 : spammy iterative messages
    // 4 : sub-loop iteration messages (for critical debugging only)

double timestampTemp = 0;
double timestampAfter = 0;


int main (void)
{
    //Initialize output outputJSON
    outputJSON  = malloc(5000);
    strcat(outputJSON, "[");
    printf("%s", DEBUG_TRUE?"Scripted started successfully\n":"");

    while(1) {

        if (JSONcount != 0 && (get_datetime() >= lastProcessedTimestamp + 7)) {
            printf("%s", DEBUG_TRUE>=2?"Conditions for sending fulfilled\n":"");

            strcat(outputJSON, "]\0");
            if (DEBUG_TRUE){
                printf("Sent %d JSON objects.\n", JSONcount);
                printf("Last processed timestamp: %f\n", lastProcessedTimestamp);
            }
            lastProcessedTimestamp = get_datetime();
            post();
            free(outputJSON);
            outputJSON = malloc(5000);
            for(int i = 0; i < 5000; i++)
                outputJSON[i] = '\0';
            strcat(outputJSON, "[");
            JSONcount = 0;
        }
        else
        {
            timestampTemp = 0;
            mainLogic();
            timestampAfter = timestampTemp;
        }
    }
}

void mainLogic(void)
{
    FILE * fp;
    char * line = NULL;
    int is_station = 0;
    size_t len = 0;
    ssize_t read;

    //Initialize to current time
    double current_time = get_datetime();


    //Read File
    fp = fopen("/triangl-package-updater/test.csv-01.csv", "r");
    //fp = fopen("/pull-latest-ipk/test.csv", "r");
    if (fp == NULL)
    {
        printf("%s", DEBUG_TRUE?"FAIL, empty file descriptor\n":"");
        sleep(2);
        //exit(EXIT_FAILURE);
        return;
    }

   // printf("mainlogic running \n");

    //Read File line by line
    while ((read = getline(&line, &len, fp)) != -1)
    {
        printf("%s%s", DEBUG_TRUE==4?"Read line:":"",DEBUG_TRUE==4?line:"");
        //skip lines untill clients get reached to set checkflag
        if(-44 == strcmp("Station MAC", line))
        {
            printf("%s", DEBUG_TRUE>=2?"Found Stations\n":"");
            is_station = 1;
            continue;
        }

        if(is_station)
            split_input(line);
    }
    printf("%s%s%s", DEBUG_TRUE>=3?"Creation of following JSON completed:\n":"",DEBUG_TRUE>=3?outputJSON:"",DEBUG_TRUE>=3?"\n":"");
    //printf("%s", outputJSON);

    fclose(fp);
    if (line)
        free(line);
    //exit(EXIT_SUCCESS);
}


char *split_input(char *line)
{
    int i = 0;
    // check for errors
    char *tok = line, *end = line;
    while (tok != NULL)
    {
        strsep(&end, ",");
        //RemoveSpaces(tok);

        //Filter Input
        // 0 Station MAC, (deviceId)
        // 1 First time seen
        // 2 Last time seen (timestampString)
        // 3 Power (signalStrength)
        // 4 # packets
        // 5 BSSID (routerId)
        // 6 Probed ESSIDs
        printf("%s%s%s", DEBUG_TRUE==4?"Current snippet in processing:\n":"",DEBUG_TRUE>=3?tok:"",DEBUG_TRUE==4?"\n":"");
        switch(i) {
            case 0 :
                if (strlen(tok) != 17){
                    i += 8;
                    break;
                }
                if(JSONcount++ > 0)
                    strcat(outputJSON, ",\n");
                strcat(outputJSON, "{\n\"deviceId\" : \"");
                strcat(outputJSON, tok);
                strcat(outputJSON, "\",");
                printf("%s%s%s", DEBUG_TRUE==4?"current JSON:\n":"",DEBUG_TRUE==4?outputJSON:"",DEBUG_TRUE>=2?"\n":"");
                i++;
                break;
            case 2:
                if(date_to_double(tok) > timestampTemp)
                    timestampTemp = date_to_double(tok);
                // Validate if newer last seen timestamp than last processed
                if(date_to_double(tok) <= timestampAfter)
                {
                    removeJSONEntry();
                    i += 5;
                    JSONcount--;
                    break;
                }
                trim(tok);
                strcat(outputJSON, "\n\"timestampString\" : \"");
                strcat(outputJSON, tok);
                strcat(outputJSON, "\",");
                i++;
                break;
            case 3 :
                strcat(outputJSON, "\n\"signalStrength\" : ");
                strcat(outputJSON, tok);
                strcat(outputJSON, ",");
                i++;
                break;
            case 5 :
                RemoveSpaces(tok);
                strcat(outputJSON, "\n\"routerId\" : \"");
                strcat(outputJSON, tok);
                strcat(outputJSON, "\"\n}");
                i++;
                break;
            default :
                i++;
        }
        tok = end;
    }
    return 0;
}

void removeJSONEntry(void)
{
    int i = 0;
    while(outputJSON[i] != '\0') {
        i++;
    }
    outputJSON[i - (i == 36 ? 35 : 37)] = '\0'; //Todo: Hardcoded length of Station Mac string
    printf("%s%s%s", DEBUG_TRUE>=3?"**Removed JSON:**   ":"",DEBUG_TRUE>=3?(outputJSON-(i == 36 ? 35 : 37)):"",DEBUG_TRUE>=3?"\n":"");
}

double date_to_double(char *str)
{
    char num[15];
    num[14] = '\0';
    int i = 0;
    int j = 0;
    int length = 19;
    while(i <= length)
    {
        if (str[i] != '-' && str[i] != ':' && str[i] != ' ')
            num[j++] = str[i];
        i++;
    }
    double d_num = atof(num);
    return (d_num);
}

double get_datetime(void)
{
    char curr_time[15];
    curr_time[14] = '\0';
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strcpy(curr_time,     itoa(tm.tm_year + 1900));
    strcpy(curr_time + 4, itoa(tm.tm_mon + 1));
    strcpy(curr_time + 6, itoa(tm.tm_mday));
    strcpy(curr_time + 8, itoa(tm.tm_hour));
    strcpy(curr_time + 10, itoa(tm.tm_min));
    strcpy(curr_time + 12, itoa(tm.tm_sec));

    printf("%s",DEBUG_TRUE==4?curr_time:"");
    return (double) atof(curr_time);
}


//Problem when clock hits zero at the end
char	*itoa(int nb)
{
    char	*str;
    int		i;
    int		nb_cpy;
    i = 0;
    str = (char*)malloc(11);
    nb_cpy = nb;
    while (nb_cpy || !i)
    {
        nb_cpy /= 10;
        str[i++] = '\0';
    }
    str[i] = '\0';
    while (i-- > 0)
    {
        str[i] = '0' + nb % 10;
        nb /= 10;
    }
    //if (strcmp(str, "0") == 0) //Todo: Fix it
    //    return ("00");

    // str[4] = '\0';
    if (DEBUG_TRUE == 4)
        printf("itoa STR out: %s", str);
    return (str);
}

void RemoveSpaces(char* source)
{
    char* i = source;
    char* j = source;
    while(*j != 0)
    {
        *i = *j++;
        if(*i != ' ')
            i++;
    }
    *i = 0;
}

void trim(char *str)
{
    int i;
    int begin = 0;
    int end = strlen(str) - 1;

    while (isspace((unsigned char) str[begin]))
        begin++;

    while ((end >= begin) && isspace((unsigned char) str[end]))
        end--;

    // Shift all characters back to the start of the string array.
    for (i = begin; i <= end; i++)
        str[i - begin] = str[i];

    str[i - begin] = '\0'; // Null terminate string.
}

int post(void)
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *header = NULL;
    header = curl_slist_append(header, "Content-Type: application/json");
    curl_slist_append(header, "charsets: utf-8");

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    curl = curl_easy_init();
    if(curl) {
        /* First set the URL that is about to receive our POST. This URL can
           just as well be a https:// URL if that is what should receive the
           data. */
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.triangl.io/tracking-ingestion-service/tracking/multiple");
        /* Now specify the POST data */
        //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");

        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, outputJSON);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcrp/0.1");
        printf("%s%s%s", DEBUG_TRUE?"Following JSON posted:\n":"",DEBUG_TRUE?outputJSON:"",DEBUG_TRUE?"\n":"");
        
        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failedanield: %s\n",
                    curl_easy_strerror(res));
        if (DEBUG_TRUE == 2)
            printf("JSON sended to server sucessfully, response-code: %d\n",res);
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return 0;
}
