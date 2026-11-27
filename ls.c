#include<stdio.h>
#include<sys/stat.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/ioctl.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>
#include<locale.h>
#include<limits.h>
#include<pwd.h>
#include<grp.h>
DIR* open_file(const char*path){
    DIR*dir=opendir(path);
    if(dir==NULL){
        perror("opendir");
        return NULL;
    }
    return dir;
}
//ls_a
void ls_a(DIR*dir){
    struct dirent*entry;
    while((entry=readdir(dir))!=NULL){
        printf("%s ",entry->d_name);
    }
    printf("\n");
    closedir(dir);
}

//ls_i
void ls_i(DIR*dir){
    struct dirent*entry;
    char geshi[1000][1000];
    int count=0;
    int len=0;
    int maxlen=0;
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
    if(maxlie<1){
        maxlie=1;
    }
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
    char display_str[1024];
    time_t mtime;
    struct stat st;
}FileInfo;
int shoujiwenjian(DIR*dir,const char*path,FileInfo files[1000],int hidden){
    struct dirent*entry;
    int count=0;
    while((entry=readdir(dir))!=NULL){
        if(!hidden&&entry->d_name[0]=='.'){
            continue;
        }
        if(count>=1000){
            break;
        }
        char fullpath[PATH_MAX];
        int n=snprintf(fullpath,sizeof(fullpath),"%s/%s",path,entry->d_name);
        if(n<0||(size_t)n>=sizeof(fullpath)){
            fprintf(stderr,"路径太长：%s\n",entry->d_name);
            continue;
        } 
        struct stat st;
        if(lstat(fullpath,&st)==-1){
            perror(fullpath);
            continue;
        }
        n=snprintf(files[count].display_str,
                     sizeof(files[count].display_str),"%s",entry->d_name);
        if (n < 0 ||(size_t)n >= sizeof(files[count].display_str)){
            fprintf(stderr, "文件名太长：%s\n", entry->d_name);
            continue;
        }
        files[count].st=st;
        files[count].mtime=st.st_mtime;
        count++;
    }
    return count;
}
  //bijiaoshijian
int cmp_time(const void *a,const void*b){
    FileInfo*file_a=(FileInfo*)a;
    FileInfo*file_b=(FileInfo*)b;
    if(file_a->mtime>file_b->mtime)return -1;
    if(file_a->mtime<file_b->mtime)return 1;
    return 0;
}
int cmp_file(const void*a,const void*b){
    const FileInfo*fa=a;
    const FileInfo*fb=b;
    return strcoll(fa->display_str,fb->display_str);//本来strcmp，与ls-r不否，改为strcoll
}
void ls_sorted(DIR*dir,const char*path,int hidden,int sort_time,int reverse,int inode,int blocks){
    FileInfo files[1000];
    int count=shoujiwenjian(dir,path,files,hidden);
    closedir(dir);
    if(count==0){
        return;
    }
    
    //paixu
    if(sort_time){
        qsort(files,count,sizeof(FileInfo),cmp_time);
    }else{
        qsort(files,count,sizeof(FileInfo),cmp_file);
    }
    if(reverse){
        for(int left=0,right=count-1;left<right;left++,right--){
            FileInfo temp=files[left];
            files[left]=files[right];
            files[right]=temp;
        }
    }
    int maxlen=0;
    long total_blocks=0;
    for(int i=0;i<count;i++){
        total_blocks+=files[i].st.st_blocks;
        if(inode||blocks){
        char name[1024];
        snprintf(name,sizeof(name),"%s",files[i].display_str);
        long file_blocks=(files[i].st.st_blocks+1)/2;
        int n;
        if(inode&&blocks){
            n=snprintf(files[i].display_str,sizeof(files[i].display_str),"%llu %ld %s",(unsigned long long)files[i].st.st_ino,file_blocks,name);
        }
        else if(inode){
            n=snprintf(files[i].display_str,sizeof(files[i].display_str),"%llu %s",(unsigned long long)files[i].st.st_ino,name);}
        else{
            n=snprintf(files[i].display_str,sizeof(files[i].display_str),"%ld %s",file_blocks,name);
        }
            if(n < 0||(size_t)n>=sizeof(files[i].display_str)) {
            fprintf(stderr,"内容太长%s\n",name);
            return;
        }
    }
    int len=strlen(files[i].display_str);
        if(len>maxlen){
            maxlen=len;
        }
    if(blocks){
        printf("总计 %ld\n",(total_blocks+1)/2);
    }
    }
        
    //shuchu
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_col=w.ws_col;
    if(term_col==0){
        term_col=80;
    }
    int maxkuan=maxlen+2;
    int maxlie=term_col/maxkuan;
    if(maxlie<1){
        maxlie=1;
    }
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

//ls_r
typedef struct{
    char str_r[512];
}File_r;
/*int cmp_file(const void*a,const void*b){
    const File_r*fa=(const File_r*)a;
    const File_r*fb=(const File_r*)b;
    return strcoll(fb->str_r,fa->str_r);//本来strcmp，与ls-r不否，改为strcoll
}*/
void ls_r(DIR*dir,const char*path){
    struct dirent*entry;
    File_r file_rs[1000];
    int count=0;
    int len=0;
    int maxlen=0;
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0){
            continue;
        }
        if(entry->d_name[0]=='.')continue;
        if(count>=1000)break;
        //AI补充边界
        snprintf(file_rs[count].str_r,sizeof(file_rs[count].str_r),"%s",entry->d_name);
        len=strlen(file_rs[count].str_r);
        if(len>maxlen){
           maxlen=len;
        }
        count++;
    }
    closedir(dir);
    if(count==0)return;
    qsort(file_rs,count,sizeof(File_r),cmp_file);
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int term_col=w.ws_col;
    if(term_col==0){
        term_col=80;
    }
    int maxkuan=maxlen+2;
    int maxlie=term_col/maxkuan;
    if(maxlie==0)maxlie=1;
    int maxhang=count/maxlie;
    if(count%maxlie!=0){
        maxhang++;
    }
    for(int i=0;i<maxhang;i++){
        for(int j=0;j<maxlie;j++){
            int index=j*maxhang+i;
            if(index<count){
            printf("%-*s",maxkuan,file_rs[index].str_r);
        }
        }
        if(i<count){
        printf("\n");
    }
    }
}

//ls_s
typedef struct{
    char str_s[256];
    long blocks;
}File_s;

void ls_s(DIR*dir,const char*path){
    struct dirent*entry;
    struct stat st;
    char fullpath[PATH_MAX];
    File_s *files=NULL;
    int count=0;
    int capacity=16;
    files=malloc(capacity*sizeof(File_s));
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0){
            continue;
        }
        if(entry->d_name[0]=='.')continue;
        snprintf(fullpath,sizeof(fullpath),"%s/%s",path,entry->d_name);
        if(lstat(fullpath,&st)==-1){
        perror("lstat");
        continue;
        }
        if(count>=capacity){
            capacity*=2;
            files=realloc(files,capacity*sizeof(File_s));
        }
        strncpy(files[count].str_s,entry->d_name,sizeof(files[count].str_s)-1);
        files[count].blocks=st.st_blocks/2;
        count++;
    }
    closedir(dir);
    int max_kuan=0;
    long sum=0;
    for(int i=0;i<count;i++){
        int len=snprintf(NULL,0,"%ld",files[i].blocks);
        if(len>max_kuan){
            max_kuan=len;
        }
    }
    for(int i=0;i<count;i++){
        sum+=files[i].blocks;
    }
    if(count>0){
        printf("总计 %ld\n",sum);
    }
    for(int i=0;i<count;i++){
        
        printf("%*ld %s\n",max_kuan,files[i].blocks,files[i].str_s);
    }
    free(files);
}

//ls_l
//zhegewobuhuixieha
void zhuanhuanqi(mode_t mode,char str[]){
    strcpy(str,"----------");
    if(S_ISDIR(mode))str[0]='d';
    if(S_ISCHR(mode))str[0]='C';
    if(S_ISBLK(mode))str[0]='b';
    if(S_ISLNK(mode))str[0]='l';

    if (mode&S_IRUSR) str[1]='r';
    if (mode&S_IWUSR) str[2]='w';
    if (mode&S_IXUSR) str[3]='x';

    if (mode&S_IRGRP) str[4]='r';
    if (mode&S_IWGRP) str[5]='w';
    if (mode&S_IXGRP) str[6]='x';

    if (mode&S_IROTH) str[7]='r';
    if (mode&S_IWOTH) str[8]='w';
    if (mode&S_IXOTH) str[9]='x';
}
void ls_l(DIR*dir,const char*path){
    struct dirent*entry;
    struct stat st;
    char fullpath[PATH_MAX];
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0){
            continue;
        }
        if(entry->d_name[0]=='.')continue;
        snprintf(fullpath,sizeof(fullpath),"%s/%s",path,entry->d_name);
        if(lstat(fullpath,&st)==-1){
        perror("lstat");
        continue;
        }
        char mode_str[11];
        zhuanhuanqi(st.st_mode,mode_str);
        char time_str[64];
        struct tm*tm_info=localtime(&st.st_mtime);
        strftime(time_str,sizeof(time_str),"%b %d %H:%M",tm_info);
        struct passwd*pw=getpwuid(st.st_uid);
        struct group*gr=getgrgid(st.st_gid);
        char *user=pw?pw->pw_name:"buzhidao";
        char *group=gr?gr->gr_name:"buzhidao";
        printf("%s %3ld %-8s %-8s %6ld %s %s\n",mode_str,(long)st.st_nlink,user,group,(long)st.st_size,time_str,entry->d_name);
        
    }
    closedir(dir);
} 

//ls_R
void ls_R(const char*path){
    DIR*dir=opendir(path);
    if(dir==NULL){
        perror("opendir");
    }
    struct dirent*entry;
    struct stat st;
    char fullpath[PATH_MAX];
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0){
            continue;
        }
        if(entry->d_name[0]=='.')continue;
        
        if(strcmp(path,"/")==0){
            snprintf(fullpath,sizeof(fullpath),"/%s",entry->d_name);//aifuzhu
        }else{
            snprintf(fullpath,sizeof(fullpath),"%s/%s",path,entry->d_name);
        }
        if(lstat(fullpath,&st)==-1){
        perror("lstat");
        continue;
        }
        if(S_ISDIR(st.st_mode)){
            printf("%s\n",fullpath);
            ls_R(fullpath);
        }
        else{
            printf("%s\n",fullpath);
        }
    }
    closedir(dir);
}

int main(int argc,char*argv[]){
    setlocale(LC_COLLATE,"");
    int opt;
    int flag_a=0;
    int flag_l=0;
    int flag_t=0;
    int flag_r=0;
    int flag_i=0;
    int flag_s=0;
    int flag_R=0;
    while((opt=getopt(argc,argv,"altrisR"))!=-1){
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
    DIR *fd=open_file(path);
    if(fd==NULL){
        return 1;
    }
    if(flag_R){
        ls_R(path);
        return 0;
    }
    if(flag_t||flag_r||flag_i){
        ls_sorted(fd,path,flag_a,flag_t,flag_r,flag_i,flag_s);
    }else if(flag_a){
        ls_a(fd);
    }/*else if(flag_l){
        ls_l(fd,path);
    }else if(flag_r){
        ls_r(fd,path);
    }else if(flag_i){
        ls_i(fd);
    }else if(flag_s){
        ls_s(fd,path);
    }else{
        ls_default(fd);
    }*/
    return 0;
}