#include<stdio.h>
#include<sys/stat.h>
#include<unistd.h>
#include<dirent.h>

//ls_a
void ls_a(const char*path){
    DIR*dir=opendir(path);
    struct dirent*entry;
    if(dir==NULL){
        perror("opendir");
        return;
    }
    while((entry=readdir(dir))!=NULL){
        printf("%s ",entry->d_name);
    }
    printf("\n");
    closedir(dir);
}




int main(int argc,char*argv[]){
    int opt;
    while((opt=getopt(argc,argv,"altrisR"))!=-1){
        switch(opt){
            case'a':
                ls_a(optind<argc?argv[optind]:".");
                break;
            case'l':
            case't':
            case'r':
            case'i':
            case's':
            case'R':

        }
    }



    return 0;
}