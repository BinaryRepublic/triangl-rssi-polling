#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <curl/curl.h>

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
char *MAC=NULL;

double timestampTemp = 0;
double timestampAfter = 0;


int main (void)
{
    //Initialize output outputJSON
    outputJSON  = malloc(5000);
    strcat(outputJSON, "[");
    FILE *mac_fd = fopen("/triangl-package-updater/my_mac", "r");
    size_t len = 0;
    if (mac_fd == NULL)
    {
        printf("FAIL, empty file descriptor. Is there a file with the mac address");
        sleep(2);
        //exit(EXIT_FAILURE);
        return(1);
    }
    getline(&MAC, &len, mac_fd);
    printf("%s", MAC);
    fclose(mac_fd);

    while(1) {

        if (JSONcount != 0 && (get_datetime() >= lastProcessedTimestamp + 7)) {
            printf("triggered sending\n");

            strcat(outputJSON, "]\0");
            printf("Sent %d JSON objects.\n", JSONcount);
            printf("Last processed timestamp: %f\n", lastProcessedTimestamp);
            lastProcessedTimestamp = get_datetime();
            post();
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

    //Initialize to current time
    double current_time = get_datetime();

    //Read File
    printf("opening csv \n");
    fp = fopen("/triangl-package-updater/test.csv-01.csv", "r");
    //fp = fopen("/pull-latest-ipk/test.csv", "r");
    if (fp == NULL)
    {
        printf("FAIL, empty file descriptor\n");
        sleep(2);
        //exit(EXIT_FAILURE);
        return;
    }

    //Read File line by line
    while ((getline(&line, &len, fp)) > 0)
    {
        //skip lines untill clients get reached to set checkflag
        if(-44 == strcmp("Station MAC", line))
        {
            printf("Set is_Station flag\n");
            is_station = 1;
            continue;
        }

        if(is_station)
            split_input(line);
    }

    //printf("%s", outputJSON);

    fclose(fp);
    if (line)
        free(line);
    //exit(EXIT_SUCCESS);
}


char *split_input(char *line)
{
    printf("split input triggered\n");
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

        switch(i) {
            case 0 :
                printf("case 0\n");
                if (strlen(tok) != 17){
                    i += 8;
                    break;
                }
                if(JSONcount++ > 0)
                    strcat(outputJSON, ",\n");
                strcat(outputJSON, "{\n\"deviceId\" : \"");
                strcat(outputJSON, tok);
                strcat(outputJSON, "\",");
                i++;
                break;
            case 2:
                printf("case 2\n");
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
                printf("case 3\n");
                strcat(outputJSON, "\n\"signalStrength\" : ");
                strcat(outputJSON, tok);
                strcat(outputJSON, ",");
                i++;
                break;
            case 5 :
                printf("case 5\n");
                RemoveSpaces(tok);
                strcat(outputJSON, "\n\"routerId\" : \"");
                strcat(outputJSON, MAC);
                strcat(outputJSON, "\",");
                
                strcat(outputJSON, "\n\"associatedAP\" : \"");
                strcat(outputJSON, tok);
                strcat(outputJSON, "\"\n}");
                i++;
                break;
            default :
                printf("case default\n");
                i++;
        }
        tok = end;
    }
    return 0;
}

void removeJSONEntry(void)
{
    printf("removeJSONEntry \n");
    int i = 0;
    while(outputJSON[i] != '\0') {
        i++;
    }
    outputJSON[i - (i == 36 ? 35 : 37)] = '\0'; //Todo: Hardcoded length of Station Mac string
}

double date_to_double(char *str)
{
    printf("date_to_double triggr \n");
    char num[15];
    num[14] = '\0';
    int i = 0;
    int j = 0;
    int length = 13;
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
    printf("get_datetime triggr \n");
    char curr_time[15];
    curr_time[14] = '\0';
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char *tm_year = itoa(tm.tm_year + 1900);
    char *tm_month = itoa(tm.tm_mon + 1);
    char *tm_mday = itoa(tm.tm_mday);
    char *tm_hour = itoa(tm.tm_hour);
    char *tm_min = itoa(tm.tm_min);
    char *tm_sec =itoa(tm.tm_sec);
    strcpy(curr_time, tm_year);
    strcpy(curr_time + 4, tm_month);
    strcpy(curr_time + 6, tm_mday);
    strcpy(curr_time + 8, tm_hour);
    strcpy(curr_time + 10, tm_min);
    strcpy(curr_time + 12, tm_sec);
    free(tm_year);
    free(tm_month);
    free(tm_mday);
    free(tm_hour);
    free(tm_min);
    free(tm_sec);

    return (double) atof(curr_time);
}


//Problem when clock hits zero at the end
char	*itoa(int nb)
{
    printf("itoa triggr \n");
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
    str[4] = '\0';
    return (str);
}

void RemoveSpaces(char* source)
{
    printf("itoa triggr \n");
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
        printf("%s\n", outputJSON);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failedanield: %s\n",
                    curl_easy_strerror(res));
        printf("SUCCESS CAPSLOCK\n");
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return 0;
}
