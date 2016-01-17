//#include <curl/curl.h>
#include <utility.h>
#include <ansi_c.h>
#include <stdlib.h>

#include "json.h"
//#include "log.h"
//#include "buf.h"

#define BUFFER_SIZE 32768
#define JSON_TOKENS 1024

/*static size_t fetch_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    buf_t *buf = (buf_t *) userp;
    size_t total = size * nmemb;

    if (buf->limit - buf->len < total)
    {
        buf = buf_size(buf, buf->limit + total);
        log_null(buf);
    }

    buf_concat(buf, buffer, total);

    return total;
}

char * json_fetch(char *url)
{
    CURL *curl = curl_easy_init();
    log_null(curl);

    curl_easy_setopt(curl, CURLOPT_URL, url);

    buf_t *buf = buf_size(NULL, BUFFER_SIZE);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fetch_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "jsmn-example (https://github.com/alisdair/jsmn-example, alisdair@mcdiarmid.org)");

    struct curl_slist *hs = curl_slist_append(NULL, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        log_die("curl_easy_perform failed: %s", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
    curl_slist_free_all(hs);

    char *js = buf_tostr(buf);
    free(buf->data);
    free(buf);

    return js;
}
*/
jsmntok_t * json_tokenise(char *js, size_t len)
{
    jsmn_parser parser;
    jsmn_init(&parser);

    unsigned int n = JSON_TOKENS;
    jsmntok_t *tokens = calloc(n, sizeof(jsmntok_t));

    int ret = jsmn_parse(&parser, js, len, tokens, n);

    while (ret == JSMN_ERROR_NOMEM)
    {
        n = n * 2 + 1;
        tokens = realloc(tokens, sizeof(jsmntok_t) * n);
        ret = jsmn_parse(&parser, js, len, tokens, n);
    }

	return tokens;
}

bool json_token_streq(char *js, jsmntok_t *t, char *s)
{
	//DebugPrintf("%s", js + t->start);
    return (strncmp(js + t->start, s, t->end - t->start) == 0
            && strlen(s) == (size_t) (t->end - t->start));
}

char * json_token_tostr(char *js, jsmntok_t *t)
{
    js[t->end] = '\0';
    return js + t->start;
}
