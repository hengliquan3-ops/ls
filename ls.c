#include<stdio.h>
#include<sys/stat.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/ioctl.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>
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

//ls_i
void ls_i(const char*path){
    DIR*dir=opendir(path);
    struct dirent*entry;
     if(dir==NULL){
        perror("opendir");
        return;
    }
    char geshi[1000][1000];
    int count=0;
    int len=0;
    int maxlen;
    while((entry=readdir(dir))!=NULL){
        sprintf(geshi[count],"%ld %s",entry->d_ino,entry->d_name);
        len=strlen(geshi[count]);
        if(len>maxlen){
            maxlen=len;
        }
        count++;
    }
    closedir(dir);
    //获取终端每一行实时有多大
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_col=w.ws_col;
    if(term_col==0){
        term_col=80;
    }
    //how kuan chang
    int maxkuan=maxlen+2;
    int maxlie=term_col/maxkuan;
    int maxhang=(count/maxlie*maxlie<count?count/maxlie+1:count/maxlie);
    //shuchu
    for(int i=0;i<maxhang;i++){
        for(int j=0;j<maxlie;j++){
            int index=j*maxhang+i;//竟然是按列优先吗
            if(index<count){
            printf("%-*s",maxkuan,geshi[index]);
        }
        }
        printf("\n");
    }
}

//ls_t
typedef struct{
    char display_str[512];
    time_t mtime;
}FileInfo;
int cmp(const void *a,const void*b){
    FileInfo*file_a=(FileInfo*)a;
    FileInfo*file_b=(FileInfo*)b;
    if(file_a->mtime>file_b->mtime)return -1;
    if(file_a->mtime<file_b->mtime)return 1;
    return 0;
}
void ls_t(const char*path){
    DIR*dir=opendir(path);
    struct dirent*entry;
     if(dir==NULL){
        perror("opendir");
        return;
    }
    FileInfo files[1000];
    int count=0;
    int len=0;
    int maxlen=0;
    while((entry=readdir(dir))!=NULL){
        //pinlianjie
        char lianjie[1000];
        sprintf(lianjie,"%s/%s",path,entry->d_name);
        struct stat st; 
        if(stat(lianjie,&st)==-1){
            continue;
        }
        //cun bian hao
        snprintf(files[count].display_str,sizeof(files[count].display_str),/*"%ld %s"*/"%s",/*entry->d_ino,*/entry->d_name);
        //cunshijian
        files[count].mtime=st.st_mtime;
        len=strlen(files[count].display_str);
        if(len>maxlen){
            maxlen=len;
        }
        count++;
    }
    closedir(dir);
    //paixu
    qsort(files,count,sizeof(FileInfo),cmp);
    //shuchu
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_col=w.ws_col;
    if(term_col==0){
        term_col=80;
    }
    int maxkuan=maxlen+2;
    int maxlie=term_col/maxkuan;
    int maxhang=(count/maxlie*maxlie<count?count/maxlie+1:count/maxlie);
    for(int i=0;i<maxhang;i++){
        for(int j=0;j<maxlie;j++){
            int index=j*maxhang+i;
            if(index<count){
            printf("%-*s",maxkuan,files[index].display_str);
        }
        }
        printf("\n");
    }
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
                ls_t(optind<argc?argv[optind]:".");
                break;
            case'r':
                ls_r(optind<argc?argv[optind]:".");
                break;
            case'i':
                ls_i(optind<argc?argv[optind]:".");
                break;
            case's':
            case'R':

        }
    }



    return 0;
}