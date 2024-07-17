#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/ioctl_mem"
#define IOCTL_ALLOCATE_MEMORY 100   // ioctl 申请内存空间命令
#define IOCTL_READ_MEMORY     101   // ioctl 读内存空间命令
#define IOCTL_WRITE_MEMORY    102   // ioctl 写内存空间命令

// 从用户态读数据写入内核态
void write_file_to_device(int fd, const char *file_path, size_t size) {
    int in_fd = open(file_path, O_RDONLY);
    if (in_fd < 0) {
        perror("Failed to open file");
        exit(-1);
    }

    char *buffer = malloc(size);
    if (NULL == buffer) {
        perror("Failed to allocate buffer");
				close(in_fd);
        exit(-1);
    }

		read(in_fd, buffer, size);
		close(in_fd);

    if (ioctl(fd, IOCTL_WRITE_MEMORY, buffer) == -1) {
        perror("Failed to write to device");
    }

    free(buffer);
}

// 从内核态读数据写入用户态
void read_device_to_file(int fd, const char *file_path, size_t size) {
    char *buffer = malloc(size);
    if (!buffer) {
        perror("Failed to allocate buffer");
        exit(-1);
    }

    if (ioctl(fd, IOCTL_READ_MEMORY, buffer) == -1) {
        perror("Failed to read from device");
        free(buffer);
        exit(-1);
    }

    int out_fd = open(file_path, O_WRONLY);
    if (out_fd < 0) {
        perror("Failed to open output file");
        free(buffer);
        exit(-1);
    }

    write(out_fd, buffer, size);
    close(out_fd);
    free(buffer);
}

int main(int argc, char *argv[])
{
	int fd, file_size;
	const char *input_file = "input.txt";
	const char *output_file = "output.txt";

	// 获取要写入文件的大小
	fd = open(input_file, O_RDONLY);
	if (fd < 0) {
		perror("open file error!");
		exit(-1);
	}
	file_size = lseek(fd, 0, SEEK_END);
	if (file_size < 0) {
		perror("get file size error!");
		goto ERROR;
	}
	close(fd);

	// 打开设备文件
	fd = open(DEVICE_PATH, O_RDWR);
	if (fd < 0) {
		perror("open device file error!");
		exit(-1);
	}
	
	// 开辟内核空间
	if (ioctl(fd, IOCTL_ALLOCATE_MEMORY, file_size) < 0) {
		perror("allocate memory error!");
		goto ERROR;
	}

	// 将文件写入字符设备
	write_file_to_device(fd, input_file, file_size);

	// 将文件读入output文件
	read_device_to_file(fd, output_file, file_size);

	return EXIT_SUCCESS;

ERROR:
	close(fd);
	return -1;
}
