#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>

const char* AE = "/*\n\
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>\n\
This file is released under the MIT license. See README-MIT for more information.\n\
*/";

const char* Executable = "wow.exe";
const char* OutputFile = "SpellFailure.h";

#define SEARCH_TEXT "SPELL_FAILED_SUCCESS"
#define SEARCH_TEXT2 "PETTAME_INVALIDCREATURE"
#define FIRST_FAILURE 0
#define INDEX_CANTDO 173

bool reverse_pointer_back_to_string(char ** ptr, char * str)
{
    size_t slen = strlen(str);
    size_t i;
    for (;;)
    {
        while ((*ptr)[0] != str[0])
        {
            (*ptr)--;
        }

        for (i = 0; i < slen; ++i)
            if ((*ptr)[i] != str[i])
                break;

        if (i == slen)
            return true;
        else
            (*ptr)--;
    }
    return false;
}
int find_string_in_buffer(char * str, size_t str_len, char * buf, size_t buf_len)
{
    char * p = buf;
    char * p_end = buf + buf_len;
    size_t remaining = buf_len;
    size_t i;

    for (;;)
    {
        while (*p != str[0] && p != p_end)
        {
            --remaining;
            ++p;
        }

        if (p == p_end)
            break;

        if (remaining < str_len)
            break;

        for (i = 0; i < str_len; ++i)
        {
            if(p[i] != str[i])
                break;
        }

        if (i == str_len)
            return (int)(p - buf);

        *p++;
    }
    return -1;
}

int main(int argc, char* argv[])
{
    FILE* in = std::fopen( Executable, "rb");
    FILE* out = std::fopen( OutputFile, "w");

    if (in == nullptr)
    {
        std::cout << "ERROR: Couldn't open %s for reading!\n";
        std::cout << "Exiting.\n";
        std::fclose(in);
        std::fclose(out);
        return -1;
    }

    if (out == nullptr)
    {
        std::cout << "ERROR: Couldn't open %s for writing!\n";
        std::cout << "Exiting.\n";
        std::fclose(in);
        std::fclose(out);
        return -1;
    }

    fseek(in, 0, SEEK_END);
    int len = ftell(in);
    fseek(in, 0, SEEK_SET);

    char * buffer = (char*)malloc(len);
    if(!buffer)
    {
        std::fclose(in);
        std::fclose(out);
        free(buffer);
        return 2;
    }

    if (fread(buffer, 1, len, in) != len)
    {
        std::fclose(in);
        std::fclose(out);
        free(buffer);
        return 3;
    }

    size_t offset = find_string_in_buffer(SEARCH_TEXT, strlen(SEARCH_TEXT), buffer, len);
    printf("Searching for `%s`...", SEARCH_TEXT);
    printf(" at %zi.\n", offset);

    if (offset < 0)
    {
        std::fclose(in);
        std::fclose(out);
        free(buffer);
        return 3;
    }
    /* dump header */
    fprintf(out, "%s", AE);
    fprintf(out, "\n\n");
    fprintf(out, "#pragma once\n\n");
    fprintf(out, "enum SpellCastResult : uint8_t\n{\n");
    std::cout << "Ripping...\n";
    char * p = (buffer + offset);
    char * name = p;
    int index = FIRST_FAILURE;
    size_t endoffset = find_string_in_buffer("SPELL_FAILED_UNKNOWN", strlen("SPELL_FAILED_UNKNOWN"), buffer, len);
    char *endp = (buffer + endoffset);
    do
    {
        // This is a terrible hack, it will most likely be incorrect later
        if (index == INDEX_CANTDO)
        {
            fprintf(out, "\t%-60s = %d,\n", "SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW",index);
            index++;
            continue;
        }

        name = p;
        fprintf(out, "\t%-60s = %d,\n", name,index);
        --p;
        if (p < endp)
            break;
        ++index;
        reverse_pointer_back_to_string(&p, "SPELL_FAILED");
    } while(true);

    // fprintf(out, "\t%-60s = %d,\n", "SPELL_CANCAST_OK",255);
    fprintf(out, "};\n");
    fprintf(out, "// #define SPELL_CANCAST_OK SPELL_FAILED_SUCCESS\n");
    fprintf(out, "\n");

    fprintf(out, "enum PetTameFailure : uint8_t\n{\n");
    offset = find_string_in_buffer(SEARCH_TEXT2, strlen(SEARCH_TEXT2), buffer, len);
    endoffset = find_string_in_buffer("PETTAME_UNKNOWNERROR", strlen("PETTAME_UNKNOWNERROR"), buffer, len);
    endp = (buffer + endoffset);
    p = (buffer + offset);
    name = p;
    index = 1;
    do
    {
        name = p;
        fprintf(out, "\t%-60s = %d,\n", name,index);
        --p;
        if (p < endp)
            break;
        ++index;
        reverse_pointer_back_to_string(&p, "PETTAME");
    } while(true);
    fprintf(out, "};\n");
    std::fclose(out);
    fclose(in);
    free(buffer);
    printf("\nDone.\n");
    return 0;
}
