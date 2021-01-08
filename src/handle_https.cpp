#include "app.h"

#include <curl/curl.h>

#include "mlib/defer.imp.h"

static size_t writefunction(void *ptr, size_t size, size_t nmemb, void *stream)
{
    fwrite(ptr, size, nmemb, (FILE *)stream);
    return (nmemb * size);
}

Handler_Status handle_https(Context *c, Manifest_Entry *e)
{
    CURL *ch;
    CURLcode rv;

    curl_global_init(CURL_GLOBAL_ALL);
    ch = curl_easy_init();

    FILE *file = fopen(e->dst, "w");
    if (!file)
    {
        c->error("Could not open destination for writing: %s\n", e->dst);
        return Handler_Status::Error;
    }

    curl_easy_setopt(ch, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(ch, CURLOPT_HEADER, 0L);
    curl_easy_setopt(ch, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(ch, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, writefunction);
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(ch, CURLOPT_HEADERFUNCTION, nullptr);
    curl_easy_setopt(ch, CURLOPT_HEADERDATA, nullptr);
    curl_easy_setopt(ch, CURLOPT_SSLCERTTYPE, "PEM");
    curl_easy_setopt(ch, CURLOPT_SSL_VERIFYPEER, 1L);

    curl_easy_setopt(ch, CURLOPT_URL, e->src);

    /* Turn off the default CA locations, otherwise libcurl will load CA
    * certificates from the locations that were detected/specified at
    * build-time
    */
    curl_easy_setopt(ch, CURLOPT_CAINFO, NULL);
    curl_easy_setopt(ch, CURLOPT_CAPATH, NULL);

    /* retrieve page without ca certificates -> should fail
    * unless libcurl was built --with-ca-fallback enabled at build-time
    */
    rv = curl_easy_perform(ch);
    defer({
        if (rv != CURLE_OK)
        {
            c->error("Could not download file\n");
        }
    });

    curl_easy_cleanup(ch);
    curl_global_cleanup();

    // printf("CA_CERTS: %s\n", CA_CERTS);
    return Handler_Status::OK;
}

