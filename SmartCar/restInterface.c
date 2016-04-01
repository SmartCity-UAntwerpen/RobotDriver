#include "restInterface.h"

bool _curlINIT = false;

size_t writeData(void* buffer, size_t size, size_t nmemb, void* userp);
size_t silentOutput(void* buffer, size_t size, size_t nmemb, void* userp);

int initRestInterface()
{
    if(_curlINIT)
    {
        //cURL already initialized
        return 1;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    _curlINIT = true;

    return 0;
}

int stopRestInterface()
{
    if(!_curlINIT)
    {
        //cURL already stopped
        return 1;
    }

    curl_global_cleanup();

    _curlINIT = false;

    return 0;
}

int httpGet(char const* url, char* message, char* errMessage)
{
    CURL* curl;
    CURLcode result;

    curl = curl_easy_init();

    if(curl == NULL)
    {
        //Could not instantiate cURL instant
        return 1;
    }

    if(curl_easy_setopt(curl, CURLOPT_URL, url) != CURLE_OK)
    {
        //cURL failed to set URL
        return 2;
    }

    //Set data write function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) message);

    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    //Perform the request
    result = curl_easy_perform(curl);

    //Close cURL instant
    curl_easy_cleanup(curl);

    if(result != CURLE_OK)
    {
        if(errMessage != NULL)
            sprintf(errMessage, "Error http-GET: %s", curl_easy_strerror(result));

        return 3;
    }

    return 0;
}

int httpPost(char const* url, char const* data, char const* header, char* errMessage)
{
    CURL* curl;
    CURLcode result;
    struct curl_slist* headerList = NULL;

    curl = curl_easy_init();

    if(curl == NULL)
    {
        //Could not instantiate cURL instant
        return 1;
    }

    if(curl_easy_setopt(curl, CURLOPT_URL, url) != CURLE_OK)
    {
        //cURL failed to set URL
        return 2;
    }

    if(header != NULL)
    {
        //Append custom header
        headerList = curl_slist_append(headerList, header);

        //Set custom header
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }

    //Append data to post field
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

    if(errMessage != NULL)
    {
        //Write respons into errMessage
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, errMessage);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
    }
    else
    {
        //Suppress server respons in console
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, silentOutput);
    }

    //Perform the request
    result = curl_easy_perform(curl);

    if(headerList != NULL)
    {
        //Free header list
        curl_slist_free_all(headerList);
    }

    //Close cURL instant
    curl_easy_cleanup(curl);

    if(result != CURLE_OK)
    {
        if(errMessage != NULL)
            sprintf(errMessage, "Error http-POST: %s", curl_easy_strerror(result));

        return 3;
    }

    return 0;
}

size_t writeData(void* buffer, size_t size, size_t nmemb, void* userp)
{
    size_t realSize = size * nmemb;
    char* message = (char*) userp;

    if(message == NULL)
    {
        //Out of memory
        printf("System out of memory! Could not allocate space for received message.\n");
        return 0;
    }

    memcpy(message, buffer, realSize);
    message[realSize] = '\0';

    return realSize;
}

size_t silentOutput(void* buffer, size_t size, size_t nmemb, void* userp)
{
    return size * nmemb;
}

