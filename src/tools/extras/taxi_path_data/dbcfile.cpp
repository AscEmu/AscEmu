#include "dbcfile.h"

DBCFile::DBCFile()
{
    data = NULL;
    stringTable = NULL;
}

DBCFile::~DBCFile()
{
    if (data)
        free(data);
    if (stringTable)
        free(stringTable);
}

bool DBCFile::open(const char * fn)
{
    FILE * pf = fopen(fn, "rb");
    if (!pf)
        return false;

    fread(header, 4, 1, pf);
    assert(header[0] == 'W' && header[1] == 'D' && header[2] == 'B' && header[3] == 'C');
    fread(&recordCount, 4, 1, pf); // Number of records
    fread(&fieldCount, 4, 1, pf); // Number of fields
    fread(&recordSize, 4, 1, pf); // Size of a record
    fread(&stringSize, 4, 1, pf); // String size

    data = (unsigned char *)malloc(recordSize*recordCount);
    stringTable = (unsigned char *)malloc(stringSize);

    if (!data || !stringTable)
    {
        fclose(pf);
        return false;
    }
    fread(data, recordSize*recordCount, 1, pf);
    fread(stringTable, stringSize, 1, pf);

    /* swap all the rows */
    fclose(pf);
    return true;
}

DBCFile::Record DBCFile::getRecord(size_t id)
{
    assert(data);
    return Record(*this, data + id*recordSize);
}

DBCFile::Iterator DBCFile::begin()
{
    assert(data);
    return Iterator(*this, data);
}
DBCFile::Iterator DBCFile::end()
{
    assert(data);
    return Iterator(*this, stringTable);
}

bool DBCFile::DumpBufferToFile(const char*fn)
{
    FILE * pFile;
    pFile = fopen(fn, "wb");
    if (!pFile)
        return false;

    //write header stuff
    fwrite((const void *)&header, 4, 1, pFile);
    fwrite((const void *)&recordCount, 4, 1, pFile);
    fwrite((const void *)&fieldCount, 4, 1, pFile);
    fwrite((const void *)&recordSize, 4, 1, pFile);
    fwrite((const void *)&stringSize, 4, 1, pFile);

    //now the main part is the data
    if (data)
        fwrite(data, recordSize*recordCount, 1, pFile);
    if (stringTable) //is it bad to have strings and no data ?
        fwrite(stringTable, stringSize, 1, pFile);

    //and pull out
    fclose(pFile);
    return true;
}

int DBCFile::AddRecord() //simply add an empty record to the end of the data section
{
    recordCount++;
    if (data)
    {
        data = (unsigned char *)realloc(data, recordCount*recordSize);
        memset(data + (recordCount - 1) * recordSize, 0, recordSize);//make sure no random values get here
    }
    else
    {
        //the dbc file is not yet opened
        printf(" Warning : adding record to an empty or unopened DBC file");
        data = (unsigned char *)malloc(recordSize);
        recordCount = 1;
    }

    //seems like an error ocured
    if (!data)
    {
        printf(" Error : Could not resize DBC data partition");
        recordCount = 0;
        return -1;
    }

    return (recordCount - 1);
}

int DBCFile::AddString(const char *new_string) //simply add an empty record to the end of the string section
{

    size_t new_str_len = strlen(new_string) + 1;

    if (new_str_len == 0)
        return 0; //we do not add 0 len strings

    if (stringTable)
        stringTable = (unsigned char *)realloc(stringTable, stringSize + new_str_len);
    else
    {
        //the dbc file is not yet opened
        printf(" Warning : adding string to an empty or unopened DBC file");
        stringTable = (unsigned char *)malloc(new_str_len);
        stringSize = (unsigned int)new_str_len;
    }

    //seems like an error ocured
    if (!stringTable)
    {
        printf(" Error : Could not resize DBC string partition");
        stringSize = 0;
        return -1;
    }

    memcpy(stringTable + stringSize, new_string, new_str_len);

    int ret = stringSize;

    stringSize += (int)new_str_len;

    return ret;
}


void DBCFile::EmptyDBC() //simply add an empty record to the end of the data section
{
    //make cleanup first
    if (data)
    {
        free(data);
        data = NULL;
    }
    if (stringTable)
    {
        free(stringTable);
        stringTable = NULL;
    }
    recordCount = 0;
    stringSize = 0;
}

void DBCFile::DumpAsCVS(char *filename)
{
    FILE *fout = fopen(filename, "w");
    if (!fout)
        return; //error

    signed int *tdata = (signed int *)data;
    for (unsigned int i = 0; i < recordCount; i++)
    {
        for (unsigned int j = 0; j < fieldCount; j++)
            fprintf(fout, "%d,", tdata[j]);
        fprintf(fout, "\n");
        tdata += recordSize / 4;
    }
    fclose(fout);
}

