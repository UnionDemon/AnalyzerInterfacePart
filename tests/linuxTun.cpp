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