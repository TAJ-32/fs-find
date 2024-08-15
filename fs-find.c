#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <errno.h>
#include <string.h>
#include </usr/src/sys/ufs/ffs/fs.h>
#include </usr/src/sys/ufs/ufs/dinode.h>
#include </usr/src/sys/ufs/ufs/dir.h>

#define INODE_SIZE 256
void search_dir(struct fs *sb, uint8_t *d_i, uint32_t inode_num, int recurse_count);
void search_simp(struct fs *sb, uint8_t *d_i, uint32_t inode_num, int recurse_count);

int main(int arg, char *argv[])
{
	int img_fd;
	struct stat filestat;
	img_fd = open(argv[1], (O_RDONLY));

	if (img_fd == -1) {
		perror("open");
		exit(1);
	}

	if (fstat(img_fd, &filestat) != 0) {
		perror("stat");
		exit(1);
	}

	uint8_t *d_i;

	d_i = mmap(0, filestat.st_size,
                       PROT_READ,
                       MAP_PRIVATE, img_fd,
                       0);

	struct fs *sb = d_i + SBLOCK_UFS2;

	ufs2_daddr_t blk_addr = ino_to_fsba(sb, 2);
	ufs2_daddr_t offset = ino_to_fsbo(sb, 2);
	struct ufs2_dinode *node = d_i + (blk_addr * sb->fs_fsize) + offset*sizeof(struct ufs2_dinode);

	int recurse_count = 0;
	if (node->di_size > sb->fs_bsize) {
		search_dir(sb, d_i, 2, recurse_count);
	}
	else {
		search_simp(sb, d_i, 2, recurse_count);
	}
	recurse_count = 0;
}

void search_dir(struct fs *sb, uint8_t *d_i, uint32_t inode_num, int recurse_count) { 

	ufs2_daddr_t blk_addr = ino_to_fsba(sb, inode_num);
	ufs2_daddr_t offset = ino_to_fsbo(sb, inode_num);
	struct ufs2_dinode *node = d_i + (blk_addr * sb->fs_fsize) + offset*sizeof(struct ufs2_dinode);
	int bytes_left = node->di_size;

	int count = 0;

	while (bytes_left >= sb->fs_bsize) {
		struct direct *dir = d_i + (sb->fs_fsize * node->di_db[count]);
		bytes_left -= dir->d_reclen;
		
		struct direct *prev = dir;
		while (bytes_left > 0) {
			struct direct *curr_dir = (char *) prev + prev->d_reclen;
			if (strcmp(curr_dir->d_name, "..") != 0) {	
				for (int i = 0; i < recurse_count; i++) {
					printf("  ");
				}
				printf("%s\n", curr_dir->d_name);

			}

			bytes_left -= curr_dir->d_reclen;
			if (curr_dir->d_type == DT_DIR && strcmp(curr_dir->d_name, ".snap") != 0 && strcmp(curr_dir->d_name, "..") != 0) {
				recurse_count += 1;
				//recursive function for directory
				uint32_t inode_num = curr_dir->d_ino;
				search_simp(sb, d_i, inode_num, recurse_count);
				recurse_count -= 1;
			}
			prev = curr_dir;

		}
		
		count++;
	}

	return;
}


void search_simp(struct fs *sb, uint8_t *d_i, uint32_t inode_num, int recurse_count) { 

	ufs2_daddr_t blk_addr = ino_to_fsba(sb, inode_num);
	ufs2_daddr_t offset = ino_to_fsbo(sb, inode_num);
	struct ufs2_dinode *node = d_i + (blk_addr * sb->fs_fsize) + offset*sizeof(struct ufs2_dinode);
	int bytes_left = node->di_size;
	struct direct *dir = d_i + (sb->fs_fsize * node->di_db[0]);
	bytes_left -= dir->d_reclen;

	struct direct *prev = dir;
	while (bytes_left > 0) {
		struct direct *curr_dir = (char *) prev + prev->d_reclen;
		if (strcmp(curr_dir->d_name, "..") != 0) {	
			for (int i = 0; i < recurse_count; i++) {
				printf("  ");
			}
			printf("%s\n", curr_dir->d_name);

		}

		bytes_left -= curr_dir->d_reclen;
		if (curr_dir->d_type == DT_DIR && strcmp(curr_dir->d_name, ".snap") != 0 && strcmp(curr_dir->d_name, "..") != 0) {
			recurse_count += 1;
			//recursive function for directory
			uint32_t inode_num = curr_dir->d_ino;
			search_simp(sb, d_i, inode_num, recurse_count);
			recurse_count -= 1;
		}
		prev = curr_dir;

	}
	return;
}


