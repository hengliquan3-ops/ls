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
        int flag_a=0;
        int flag_l=0;
        int flag_t=0;
        int flag_r=0;
        int flag_i=0;
        int flag_s=0;
        int flag_R=0;
   
        switch(opt){
            case'a':
                flag_a=1;
                break;
            case'l':
                flag_l=1;
                break;
            case't':
                flag_t=1;
                break;
            case'r':
                flag_r=1;
                break;
            case'i':
                flag_i=1;
                break;
            case's':
                flag_s=1;
                break;
            case'R':
                flag_R=1;
                break;
            default:
                break;
        }
    }
    const char*path=(optind<argc?argv[optind]:".");
    return 0;
}