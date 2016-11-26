#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <math.h>
#include <errno.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
#define MINE 0
typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, char* key_words, int type){
	EXPECT(c, *key_words);
	key_words ++;
	do{
		if (*c->json++ != *key_words++)
			return LEPT_PARSE_INVALID_VALUE;
	} while (*c->json != '\0' && *key_words != '\0');
	if (*key_words != '\0')
		return LEPT_PARSE_INVALID_VALUE;
	v->type = type;
	return LEPT_PARSE_OK;
}

#if 0
static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}
#endif

#if 0
static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}
#endif

#if 0
static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}
#endif

static int lept_parse_number(lept_context* c, lept_value* v) {
#if MINE
    char* end;
	int ret = LEPT_PARSE_OK;
    /* \TODO validate number */
    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
	if (!ISDIGIT(*c->json) && *c->json != '-')
		return  LEPT_PARSE_INVALID_VALUE;
	if (*c->json == '-')
		++c->json;
	if (*c->json == '0' && ISDIGIT(*(c->json + 1))){
		v->n = 0.0;
		return LEPT_PARSE_ROOT_NOT_SINGULAR;
	}
	if (*end != NULL)
		return LEPT_PARSE_ROOT_NOT_SINGULAR;
	while (*c->json++ != '.');
	if (*c->json == '\0')
		return LEPT_PARSE_INVALID_VALUE;
	if ((v->n == HUGE_VAL || v->n == -HUGE_VAL) && errno == ERANGE)
		return LEPT_PARSE_NUMBER_TOO_BIG;
	c->json = end;
    v->type = LEPT_NUMBER;
	return ret;
#endif
#if !MINE
	char* end;
	char* p;
	/* \TODO validate number */
	v->n = strtod(c->json, &end);
	if (c->json == end)
		return LEPT_PARSE_INVALID_VALUE;
	p = c->json;
	if (*p == '-') ++p;
	if (*p == '0') ++p;
	else {
		if (!ISDIGIT1TO9(*p))
			return LEPT_PARSE_INVALID_VALUE;
		for (++p; ISDIGIT(*p); ++p);
	}
	if (*p == '.'){
		++p;
		if (!ISDIGIT(*p))
			return LEPT_PARSE_INVALID_VALUE;
		for (++p; ISDIGIT(*p); ++p);
	}
	if (*p == 'e' || *p == 'E'){
		++p;
		if (*p == '-' || *p == '+') ++p;
		if (!ISDIGIT(*p))
			return LEPT_PARSE_INVALID_VALUE;
		for (++p; ISDIGIT(*p); ++p);
	}
	if ((v->n == HUGE_VAL || v->n == -HUGE_VAL) && errno == ERANGE)
		return LEPT_PARSE_NUMBER_TOO_BIG;
	c->json = p;
	v->type = LEPT_NUMBER;
	return LEPT_PARSE_OK;
#endif
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
		case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
		case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
