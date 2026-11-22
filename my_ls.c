#include <dirent.h>//引入目录操作接口
#include <stdio.h>

int main(int argc, char *argv[]) {
    const char *path = argc > 1 ? argv[1] : ".";
    
    DIR *dir = opendir(path);//DIR代表已打开的目录opendir()打开目录
    if (dir == NULL) {//检查是否打开成功
        perror(path);//打印错误原因
        return 1;
    }

    struct dirent *entry;//准备保存每次打开的目录项
    while ((entry = readdir(dir)) != NULL) {//不断读取目录项
        puts(entry->d_name);//打印目录项
    }

    closedir(dir);//关闭目录
    return 0;
}