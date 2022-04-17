#ifndef __CUSTOM_STRUCTS_INCLUDED__
#define __CUSTOM_STRUCTS_INCLUDED__

struct ProcConfig
{
    int proc_id = -1;
    int proc_count = -1;
};

struct MatrixCoordinates
{
    int row_start;
    int row_end;
    int column_start;
    int column_end;
};

#endif // __CUSTOM_STRUCTS_INCLUDED__
