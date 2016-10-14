#pragma once

// LICENSE
//
//   This software is dual-licensed to the public domain and under the following
//   license: you are granted a perpetual, irrevocable license to copy, modify,
//   publish, and distribute this file as you see fit.

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef uint32_t markdown_uint32;
typedef int32_t markdown_int32;

typedef enum
{
    MARKDOWN_TOKEN_TEST,
    MARKDOWN_TOKEN_HEADING1,
    MARKDOWN_TOKEN_HEADING2,
    MARKDOWN_TOKEN_HEADING3,
    MARKDOWN_TOKEN_HEADING4,
    MARKDOWN_TOKEN_HEADING5,
    MARKDOWN_TOKEN_HEADING6,
    MARKDOWN_TOKEN_THEMATIC_BREAK,
    MARKDOWN_TOKEN_PARAGRAPH,
    MARKDOWN_TOKEN_EOF
} markdown_token_type;

typedef struct markdown_token
{
    markdown_token_type type;
    markdown_uint32 length;
    char* text;
} markdown_token;

typedef struct markdown_tokenizer
{
    char* at;
} markdown_tokenizer;

static void markdown_trim_space_begin(markdown_token* token)
{
    if (token->length > 0)
    {
        while (token->text[0] == ' ')
        {
            token->text++;
            token->length--;
        }
    }
}

static void markdown_trim_whitespace_begin(markdown_token* token)
{
    if (token->length > 0)
    {
        while (token->text[0] == ' ' ||
            token->text[0] == '\n' ||
            token->text[0] == '\r' ||
            token->text[0] == '\t' ||
            token->text[0] == '\v' ||
            token->text[0] == '\r')
        {
            token->text++;
            token->length--;
        }
    }
}

static void markdown_trim_space_end(markdown_token* token)
{
    if (token->length > 0)
    {
        markdown_uint32 pos = token->length - 1;

        while (token->text[pos] == ' ')
        {
            pos--;
        }

        token->length = pos + 1;
    }
}

static void markdown_trim_whitespace_end(markdown_token* token)
{
    if (token->length > 0)
    {
        markdown_uint32 pos = token->length - 1;

        while (token->text[pos] == ' ' ||
            token->text[pos] == '\n' ||
            token->text[pos] == '\r' ||
            token->text[pos] == '\t' ||
            token->text[pos] == '\v' ||
            token->text[pos] == '\r')
        {
            pos--;
        }

        token->length = pos + 1;
    }
}

static char* markdown_normalize_line_endings(char* text)
{
    size_t length = strlen(text);

    markdown_uint32 sourceAt = 0;
    markdown_uint32 destAt = 0;

    markdown_uint32 newlinesToAdd = 0;

    //+2 for possibly inserting newlines at end.
    char* cleaned = malloc(length + 2);

    while (sourceAt < length)
    {
        if (text[sourceAt] == '\r' && text[sourceAt + 1] == '\n')
        {
            cleaned[destAt++] = '\n';
            sourceAt += 2;
        }
        else if (text[sourceAt] == '\r')
        {
            cleaned[destAt++] = '\n';
            sourceAt++;;
        }
        else
        {
            cleaned[destAt++] = text[sourceAt++];
        }
    }

    if (cleaned[destAt - 1] == '\n' && cleaned[destAt - 2] != '\n')
    {
        newlinesToAdd = 1;
    }

    if (cleaned[destAt - 1] != '\n')
    {
        newlinesToAdd = 2;
    }

    if (newlinesToAdd > 0)
    {
        cleaned[destAt++] = '\n';
        if (newlinesToAdd > 1)
        {
            cleaned[destAt++] = '\n';
        }
    }

    cleaned[destAt] = '\0';
    realloc(cleaned, destAt + 1);

    return cleaned;
}

static void markdown_parse_paragraph(markdown_tokenizer* tokenizer, markdown_token* token)
{
    markdown_uint32 charCount = 1;

    while (tokenizer->at[charCount] != '\0')
    {
        if(tokenizer->at[charCount] != '\n' || tokenizer->at[charCount + 1] != '\n')
        {
            charCount++;
        }
        else
        {
            break;
        }
    }

    token->type = MARKDOWN_TOKEN_PARAGRAPH;

    token->text = malloc(charCount + 1);
    strncpy(token->text, tokenizer->at, charCount);
    token->text[charCount + 1] = '\0';

    token->length = charCount;

    markdown_trim_whitespace_end(token);
    markdown_trim_whitespace_begin(token);

    tokenizer->at += charCount + 2;
}

static void markdown_parse_thematic_break(markdown_tokenizer* tokenizer, markdown_token* token, const char breakCharacter)
{
    token->type = MARKDOWN_TOKEN_THEMATIC_BREAK;

    markdown_uint32 breakTokenCount = 1;
    markdown_uint32 charCount = 1;

    while (tokenizer->at[charCount] != '\0' &&
        tokenizer->at[charCount] != '\n')
    {
        if (tokenizer->at[charCount] == breakCharacter)
        {
            breakTokenCount++;
            charCount++;
        }
        else if (tokenizer->at[charCount] == ' ')
        {
            charCount++;
        }
        else
        {
            markdown_parse_paragraph(tokenizer, token);
        }
    }

    if (breakTokenCount < 3)
    {
        markdown_parse_paragraph(tokenizer, token);

    }
    else
    {
        tokenizer->at += charCount + 1;
    }
}

static void markdown_parse_atx_headers(markdown_tokenizer* tokenizer, markdown_token* token)
{
    markdown_uint32 headingCounter = 1;
    markdown_uint32 charCount = 0;

    while (tokenizer->at[headingCounter] &&
        tokenizer->at[headingCounter] == '#')
    {
        headingCounter++;
    }

    if (headingCounter <= 6)
    {
        if (tokenizer->at[headingCounter] == ' ')
        {
            while (tokenizer->at[headingCounter + charCount] != '\0')
            {
                if (tokenizer->at[headingCounter + charCount] == '\n')
                {
                    break;
                }
                else
                {
                    charCount++;
                }
            }

            token->type = MARKDOWN_TOKEN_HEADING1 + (headingCounter - 1);
            token->text = &tokenizer->at[headingCounter + 1];
            //NOTE(Simon): - 1 to get rid of newline
            token->length = charCount - 1;
            //NOTE(Simon): + 1 to skip tokenizer past newline
            tokenizer->at += charCount + headingCounter + 1;

            markdown_trim_space_end(token);
            markdown_trim_space_begin(token);
        }
        else
        {
            markdown_parse_paragraph(tokenizer, token);
        }
    }
    else
    {
        markdown_parse_paragraph(tokenizer, token);
    }
}

static markdown_token markdown_get_token(markdown_tokenizer* tokenizer)
{
    markdown_token token = {0};
    token.length = 1;
    token.text = &tokenizer->at[0];

    switch(tokenizer->at[0])
    {
        case '\0':
        {
            token.type = MARKDOWN_TOKEN_EOF;
            break;
        }
        case '#':
        {
            markdown_parse_atx_headers(tokenizer, &token);
            break;
        }
        case '*':
        {
            markdown_parse_thematic_break(tokenizer, &token, '*');
            break;
        }
        case '-':
        {
            markdown_parse_thematic_break(tokenizer, &token, '-');
            break;
        }
        case '_':
        {
            markdown_parse_thematic_break(tokenizer, &token, '_');
            break;
        }

        case '\n':
        {
            while(tokenizer->at[0] == '\n')
            {
                tokenizer->at++;
            }

            token = markdown_get_token(tokenizer);
            break;
        }

        default:
        {
            markdown_parse_paragraph(tokenizer, &token);
            break;
        }
    }

    return token;
}

extern char* markdown_compile_ast(char* markdown)
{
    markdown_tokenizer tokenizer = {0};
    char* cleaned = markdown_normalize_line_endings(markdown);
    tokenizer.at = cleaned;

    bool parsing = true;

    char* result = "";

    while (parsing)
    {
        markdown_token token = markdown_get_token(&tokenizer);

        switch(token.type)
        {
            case MARKDOWN_TOKEN_EOF:
                printf("End of file");
                parsing = false;
            break;

            case MARKDOWN_TOKEN_HEADING1:
                printf("<h1>%.*s</h1>\n", token.length, token.text); 
            break;
            case MARKDOWN_TOKEN_HEADING2:
                printf("<h2>%.*s</h2>\n", token.length, token.text);
            break;
            case MARKDOWN_TOKEN_HEADING3:
                printf("<h3>%.*s</h3>\n", token.length, token.text);
            break;
            case MARKDOWN_TOKEN_HEADING4:
                printf("<h4>%.*s</h4>\n", token.length, token.text);
            break;
            case MARKDOWN_TOKEN_HEADING5:
                printf("<h5>%.*s</h5>\n", token.length, token.text);
            break;
            case MARKDOWN_TOKEN_HEADING6:
                printf("<h6>%.*s</h6>\n", token.length, token.text);
            break;

            case MARKDOWN_TOKEN_THEMATIC_BREAK:
                printf("<hr />\n");
            break;

            case MARKDOWN_TOKEN_PARAGRAPH:
                printf("<p>%.*s</p>\n", token.length, token.text);
            break;
        }
    }

    return result;
}
