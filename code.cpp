/*
void print(int a);

int myFunc() {
int* p;
int a = 10;
*p *= 2;
if (*p > 10) {
    print(*p);
    p = nullptr;
} else {
    *p *= 10;
    a -= *p;
}
return *p + 8;
}*/
/*
int myFunc() {
    int p, b, a;
    a=7;
    for (p=1; p < b; p =  p + b) 
        if (a<10)
        b++;
    
        
return b + 8;
}*/

/*
void print(int x);
int myFunc() {
    int a = 10;
    int b=5;
    for(a=1; a<=10; a++)
        b++;
    return 0;
}*/
/*
int test(int a, int b) {
if (b != 0)
a = a + 10 * a + a * 5;


return a;
}*/

/*
int nod(int first, int second)
{
	while (first != second) {
        if (first < second)
        {
            second -= first;
        }
		else
			{
               first -= second;  
            }
           
    }
		
	return first;
}*/
/*
//разыменование нулевого указателя
int main ()
{
  int firstvalue = 5, secondvalue = 15;
  int * p1, * p2;
  
  p1 = &firstvalue;
  p2 = &secondvalue;
  *p1 = 10;
  *p2 = *p1;
  p1 = p2;           
  *p1 = 20;

  return 0;
}*/
/*
//разыменование нулевого указателя
int main() {
    int a = 6;
    int *p;
    p = nullptr;
    *p = a;
}*/
//разыменование нулевого указателя+неинициализированный указатель
/*
int main() {
    int a = 6;
    int *p;
    *p = a;
}*/
/*переполнение знаковых целых типов
int main() 
{
    long int a, b, c;
    if (a+b <=0)
        return -1;
    return 0;
}*/


/*
int main() {
    int a = 6;
    int *p;
    p = &a;

    if (*p < 3)
    {
        return -1;
    }
    return 10;
}*/

//разыменование нулевого указателя
/*
int main() {
    int *p;
    int a = 6;
    if (p != nullptr) {
        return a;
    }
    *p = a;
    return a+6;
}*/
/*
int f(int* p) {
    if (p != nullptr) {
        return -1;
    } else {
        return *p + 10;
    }
}*/


// ===============================================


/*
extern void print(int x);

int f(int a, int b) {
    print(a);
    print(b);

    int sum = 0;
    for (int i = 0; i < a; i++) {
        sum += i;
    }
    sum *= b;

    return sum;
}
*/

/*
extern void print(int x);

void f(int a) {
    int sum = 0;

    for (int i = 0; i < a; i++) {
        if (i % 2 == 0) {
            sum += i;
        }
        print(sum);
    }

    print(sum);
}
*/

/*
extern void print(int a);

void f(int a) {
    int sum = 0;
    for (int i = 0; i < a; i++)
        if (i % 2 == 0)
            sum += i;
        else
            sum -= 2 * i;
    
    print(sum);
}*/
//пример из линукса на переполнение знаковых целых типов
/*
typedef int loff_t;
int vfs_fallocate(struct file *file, int mode, loff_t offset, loff_t len)
{
	struct inode *inode = file_inode(file);
    if (offset < 0 || len <= 0)
		return -EINVAL;
	 Check for wrap through zero too 
	if ((offset + len > inode->i_sb->s_maxbytes) || (offset + len < 0))
        return -EFBIG;
}
*/
//пример из линукса на разыменование нулевого и неинициализированного указателя
/*
struct sock {
  void* data;
};
 
struct tun_struct {
unsigned int 		flags;
  sock* sk;
};
 
struct tun_file {
	struct tun_struct *tun;
};
 
struct file {
  tun_file* private_data;
};
 
struct poll_table {
  void* data;
};
 
extern tun_struct* __tun_get(tun_file* file);
 
#define POLLERR -1
 
static unsigned int tun_chr_poll(struct file *file, poll_table * wait)
{
	struct tun_file *tfile = file->private_data;
	struct tun_struct *tun = __tun_get(tfile);
	
	if (tun == nullptr) {
	    return -1;
	}
	
	struct sock *sk = tun->sk;
	unsigned int mask = 0;
 
	if (!tun)
		return POLLERR;
 
	return 0;
}
*/

/*
static int podhd_try_init(struct usb_interface *interface, struct usb_line6_podhd *podhd)
{
  int err;
  struct usb_line6 *line6 = &podhd->line6;

  if ((interface == NULL) || (podhd == NULL))
    return -ENODEV;
}*/
/*
void refresh(int* frameCount)
{
    if (frameCount != nullptr) {
        ++(*frameCount); // прямо вот тут грохалась из-за разыменования nullptr
    }
}*/

/*
struct student {
    char name[80];
    int age;
    float percentage;
};
int main()
{
    struct student* emp;
    emp->age = 18;
    return 0;
}
*/

struct FILE_DESCR {
    int id;
    const char* file_name;
};
 
extern const char* read(int fd);
extern void print(const char* str);
 
void f(FILE_DESCR* fd)
{
    
    if (fd == nullptr) {
        return;
    }
    
    const char* input;
    input = read(fd->id);
 
    print(input);
}