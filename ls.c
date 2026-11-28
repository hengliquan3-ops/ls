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
void zhuanhuanqi(mode_t mode,char str[]);
void ls_sorted(DIR*dir,const char*path,int hidden,int sort_time,int reverse,int inode,int blocks,int long_format){
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
    if(long_format){
        long total_blocks=0;
        for(int i=0;i<count;i++){
            total_blocks+=files[i].st.st_blocks;
        }
        printf("总计 %ld\n",(total_blocks+1)/2);
        for(int i=0;i<count;i++){
            struct stat*st=&files[i].st;
            if(inode){
                printf("%llu ",(unsigned long long)st->st_ino);
            }
            if(blocks){
                printf("%ld ",(long)((st->st_blocks+1)/2));
            }
            char mode_str[11];
            zhuanhuanqi(st->st_mode,mode_str);
            struct passwd*pw=getpwuid(st->st_uid);
            struct group*gr=getgrgid(st->st_gid);
            char time_str[64]="?";
            struct  tm*tm_info=localtime(&st->st_mtime);
            if(tm_info!=NULL){
               strftime(time_str,sizeof(time_str),"%b %d %H:%M",tm_info);
            }
            printf("%s %3ld %-8s %-8s %8lld %s %s\n",mode_str,(unsigned long)st->st_nlink,pw?pw->pw_name:"?",gr?gr->gr_name:"?",(long long)st->st_size,time_str,files[i].display_str);
        
        }
        return;
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
int ls_walk(const char*path,int a,int t,int r,int i,int s,int l,int R){
    DIR *dir=open_file(path);
    if(dir==NULL){
        return 1;
    }
    if(R){
        printf("%s:\n",path);
    }
    ls_sorted(dir,path,a,t,r,i,s,l);
    if(!R){
        return 0;
    }
    dir=open_file(path);
    if(dir==NULL){
        return 1;
    }
    int status=0;
    struct dirent*entry;
    while((entry=readdir(dir))!=NULL){
        if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0){
            continue;
        }
        if(!a&&entry->d_name[0]=='.'){
            continue;
        }
        char fullpath[PATH_MAX];
        int n=snprintf(fullpath,sizeof(fullpath),"%s/%s",path,entry->d_name);
        if(n<0||(size_t)n>=sizeof(fullpath)){
            fprintf(stderr,"路径太长：%s\n",entry->d_name);
            status=1;
            continue;
        }
        struct stat st;
        if(lstat(fullpath,&st)==-1){
            perror(fullpath);
            status=1;
            continue;
        }
        if(S_ISDIR(st.st_mode)){
            printf("\n");
            if(ls_walk(fullpath,a,t,r,i,s,l,R)!=0){
                status=1;
            }
        }
    }
    closedir(dir);
    return status;
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
    return ls_walk(path,flag_a,flag_t,flag_r,flag_i,flag_s,flag_l,flag_R);
}