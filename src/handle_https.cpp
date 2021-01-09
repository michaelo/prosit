#include "app.h"

#include <curl/curl.h>
#include <filesystem>

#include "mlib/defer.imp.h"

namespace fs = std::filesystem;

static size_t writefunction(void *ptr, size_t size, size_t nmemb, void *stream)
{
    fwrite(ptr, size, nmemb, (FILE *)stream);
    return (nmemb * size);
}

Handler_Status handle_https(Context *c, Manifest_Entry *e)
{
    CURL *ch;
    CURLcode rv;

    // Setup
    curl_global_init(CURL_GLOBAL_ALL);
    defer(curl_global_cleanup(););
    ch = curl_easy_init();
    defer(curl_easy_cleanup(ch););

    // Setup temp-file
    // TODO: Warns (at least at macOS). Revise tmp-file handling.
    //       Will most likely be platform-dependent
    char tmp_name[L_tmpnam];
    tmpnam(tmp_name);
    FILE *file = fopen(tmp_name, "wb");
    if (!file)
    {
        c->error("Could not open temp-file for writing: %s\n", tmp_name);
        return Handler_Status::Error;
    }
    defer(remove(tmp_name));

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

    // URL to retrieve
    curl_easy_setopt(ch, CURLOPT_URL, e->src);

    // Got auth?
    char username[128];
    char password[128];
    if (extract_login_from_uri(e->src, username, sizeof(username), password, sizeof(password)))
    {
        curl_easy_setopt(ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(ch, CURLOPT_USERNAME, username);
        curl_easy_setopt(ch, CURLOPT_PASSWORD, password);
    }

    // Turn off the default CA locations, otherwise libcurl will load CA
    // certificates from the locations that were detected/specified at
    // build-time
    curl_easy_setopt(ch, CURLOPT_CAINFO, NULL);
    curl_easy_setopt(ch, CURLOPT_CAPATH, NULL);

    // Execute
    rv = curl_easy_perform(ch);
    long http_code = 0;
    curl_easy_getinfo(ch, CURLINFO_RESPONSE_CODE, &http_code);
    
    // Evaluate and possibly store resulting file
    if (rv != CURLE_OK)
    {
        c->error("Could not download file: %s\n", e->src);
        return Handler_Status::Error;
    }

    switch (http_code)
    {
        {
        case 200:
            // Store data
            std::error_code code;
            fs::rename(tmp_name, e->dst, code);
            if (code.value() != 0)
            {
                c->error("Could not download file: %s. Uanble to copy temporary file to destination.\n", e->src);
            }
            break;
        }
        {
        default:
            c->error("Could not download file: %s. HTTP code: %d\n", e->src, http_code);
            return Handler_Status::Error;
            break;
        }
    }

    return Handler_Status::OK;
}
