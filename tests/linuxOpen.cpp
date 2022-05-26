typedef int loff_t;
int vfs_fallocate(struct file *file, int mode, loff_t offset, loff_t len)
{
	struct inode *inode = file_inode(file);
    if (offset < 0 || len <= 0)
		return -EINVAL;
	if ((offset + len > inode->i_sb->s_maxbytes) || (offset + len < 0))
        return -EFBIG;
}