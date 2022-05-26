struct FILE_DESCR {
    int id;
    const char* file_name;
};
 
extern const char* read(int fd);
extern void print(const char* str);
 
void f(FILE_DESCR* fd)
{
    const char* input;
    input = read(fd->id);
 
    print(input);
}