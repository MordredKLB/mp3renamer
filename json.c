#include <utility.h>
#include <ansi_c.h>
#include <stdlib.h>

#include "json.h"

#define BUFFER_SIZE 32768
#define JSON_TOKENS 1024


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
