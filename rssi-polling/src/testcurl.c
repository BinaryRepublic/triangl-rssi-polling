#include <stdio.h>
#include "curl.h"

int main(void)
{
  CURL *curl;
  CURLcode res;
  struct curl_slist *header = NULL;
  header = curl_slist_append(header, "x-api-key: d621e2b2a7954c9985256c7d19fc8df2");
  header = curl_slist_append(header, "Content-Type: application/json");
  curl_slist_append(header, "charsets: utf-8");
  char* jsonObj = "{}";

  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    /* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. */
    curl_easy_setopt(curl, CURLOPT_URL, "https://bf94721f-c647-4a3f-b6e4-19c6c90917e5.mock.pstmn.io/signals");
    /* Now specify the POST data */
    //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonObj);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcrp/0.1");

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failedanield: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return 0;
}
