#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include </usr/src/sys/ufs/ffs/fs.h>
#include </usr/src/sys/ufs/ufs/dinode.h>
#include </usr/src/sys/ufs/ufs/dir.h>

void search_dir(struct fs *sb, uint8_t *d_i, uint32_t inode_num, int numslashes, char *directs[], int index);
void cat(uint32_t inode_num, struct fs *sb, uint8_t *d_i);

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

	//size_t size = filestat.st_size;
	printf("Size: %zu\n", filestat.st_size);

	d_i = mmap(0, filestat.st_size,
                       PROT_READ,
                       MAP_PRIVATE, img_fd,
                       0);



	struct fs *sb = d_i + SBLOCK_UFS2;

	int length = strlen(argv[2]);
	int numslashes = 0;

	printf("HI: %d\n", length);
	for (int i = 0; i < length; i++) {
		if (argv[2][i] == '/') {
			numslashes++;
		}
	}

	char *directs[numslashes + 1];

	char *tok = strtok(argv[2], "/");
	int count = 0;

	while (tok != NULL) {
		directs[count] = tok;
		tok = strtok(NULL, "/");
		count++;
	}

	search_dir(sb, d_i, 2, numslashes, directs, 0);

}

void search_dir(struct fs *sb, uint8_t *d_i, uint32_t inode_num, int numslashes, char *directs[], int index) { 

	ufs2_daddr_t root_blk_addr = ino_to_fsba(sb, inode_num);
	ufs2_daddr_t root_offset = ino_to_fsbo(sb, inode_num);
	struct ufs2_dinode *node = d_i + (root_blk_addr * sb->fs_fsize) + root_offset*sizeof(struct ufs2_dinode);
	int bytes_left = node->di_size;
	struct direct *dir = d_i + (sb->fs_fsize * node->di_db[0]);
	bytes_left -= dir->d_reclen;

	struct direct *prev = dir;
	for (int i = 0; i < numslashes + 1; i++) {
		struct direct *curr_dir = (char *) prev + prev->d_reclen;

		if (curr_dir->d_type == DT_REG) {
			uint32_t inode_num = curr_dir->d_ino;
			cat(inode_num, sb, d_i);
			return;
		}

		if (strcmp(curr_dir->d_name, directs[index]) == 0) {
			uint32_t inode_num = curr_dir->d_ino;
			index += 1;
			search_dir(sb, d_i, inode_num, numslashes, directs, index);
		}	

		prev = curr_dir;		
	}

}

void cat(uint32_t inode_num, struct fs *sb, uint8_t *d_i) {

	ufs2_daddr_t blk_addr = ino_to_fsba(sb, inode_num);
	ufs2_daddr_t offset = ino_to_fsbo(sb, inode_num);
	struct ufs2_dinode *node = d_i + (blk_addr * sb->fs_fsize) + offset*sizeof(struct ufs2_dinode);
	int bytes_left = node->di_size;
	struct direct *dir = d_i + (sb->fs_fsize * node->di_db[0]);


	char *content = d_i + (sb->fs_fsize * node->di_db[0]);

	printf("%s\n", content);

	return;

}

/*
 *	int bytes_left = node->di_size;
	struct direct *dir = d_i + (sb->fs_fsize * node->di_db[0]);
	bytes_left -= dir->d_reclen;

	struct direct *prev = dir;
	for (int i = 0; i < numslashes + 1; i++) {
		struct direct *curr_dir = (char *) prev + prev->d_reclen;

		if (curr_dir->d_type == DT_REG) {
			uint32_t inode_num = curr_dir->d_ino;
			cat(inode_num, sb, d_i);
			return;
		}

		if (strcmp(curr_dir->d_name, directs[index]) == 0) {
			uint32_t inode_num = curr_dir->d_ino;
			index += 1;
			search_dir(sb, d_i, inode_num, numslashes, directs, index);
		}	

		prev = curr_dir;		
	}

	return;
 *
 */ 
