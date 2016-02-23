#ifndef uint32
	typedef unsigned int uint32;
#endif
#ifndef int
	typedef int int32;
#endif

#ifndef ASSERT
	#define ASSERT assert
#endif

#define SERVER_DECL __declspec(dllexport)

#define SQL_INSERTS_PER_QUERY 1000

//this might change from 1 version to another of the DBC
#define COLUMN_COUNT 11
#define COLUMN_COUNT2 4
//last column is "skip_this_for_sql"

const char sql_translation_table[COLUMN_COUNT][3][200] = 
{
    { "uint32", "Id", "0" },                    //0
    { "uint32", "path", "0" },                  //1
    { "uint32", "seq", "0" },                   //2
    { "uint32", "map_id", "0" },                //3
    { "float", "position_x", "0" },             //4
    { "float", "position_y", "0" },             //5
    { "float", "position_z", "0" },             //6
    { "uint32", "flags", "0" },                 //7
    { "uint32", "waittime", "0" },              //8
    { "uint32", "arrival_event_id", "0" },      //9
    { "uint32", "departure_event_id", "0" },    //10
};

const char sql_translation_table2[COLUMN_COUNT2][3][200] =
{
    { "uint32", "Id", "0" },                    //0
    { "uint32", "from", "0" },                  //1
    { "uint32", "to", "0" },                    //2
    { "uint32", "price", "0" },                 //3
};
