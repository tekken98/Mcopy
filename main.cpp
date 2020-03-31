#include <string>
#include <cstring>
#include <list>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
using namespace std;
typedef const string CSTR;

const int BUFSIZE =  1 << 25;
double totalSize;
double finishedSize;
char *buff;
void copyFile(CSTR& srcFile,CSTR& dstFile);
string srcPath;
string dstPath;
time_t beginTime;

int getFileSize(CSTR& filename){
    struct stat64 sb;
    if (lstat64(filename.c_str(),&sb) == -1){
        //printf("%s lstat error %d  ! \n" ,filename.c_str(), errno);
        return -1;
    }
    return sb.st_size;
}
bool isDir(CSTR& path){
    struct stat sb;
    if (lstat(path.c_str(),&sb) == -1){
        cout << "lstat " << path << " error!" << endl;
        return false;
    }
    return (sb.st_mode & S_IFMT) == S_IFDIR;
}
struct MyDir{
    string dirname;
    list<MyDir*> sub_dir;
    list<pair<string,int>> file;
    void copy(CSTR& dst){
        string path;
        if (dst != ".")
            path = dst + "/" + dirname;
        else
            path = dirname;
        for (auto a : file){
            copyFile(srcPath + "/" + path + "/" + a.first, 
                     dstPath + "/" + path + "/" + a.first);
        }
        for (auto a : sub_dir){
            a->copy(path);
        }
    }
    void p(CSTR& path){
        cout <<  dirname << endl;
        for (auto a : file){
            cout << "  " << path +"/" +  dirname + "/" + a.first 
                << " [" << a.second << "]" 
                << endl;;
        }
            
        for (auto a : sub_dir)
            a->p(path + "/" + dirname);
    }
    void del(){
        for (auto a : sub_dir){
            a->del();
            //printf("delete %s \n" , a->dirname.c_str());
            delete a;
        }
    }
};
MyDir* enum_dir(CSTR& s){
    DIR * dirp = opendir(s.c_str());
    list<string> dirlist;
    if (dirp == NULL){
        printf("opendir %s error!\n", s.c_str());
        printf("erron %d %d\n",errno,EMFILE);
        return NULL;
    }
    MyDir * dirinfo = new MyDir();
    //printf("new \n");
    std::size_t found = s.rfind("/");
    string dir;
    if (found != string::npos)
        dir = s.substr(found+1);
    else
        dir = s;
    dirinfo->dirname = dir;

    dirent * direntp;
    int filesize;
    while( (direntp = readdir(dirp)) != NULL){
        if (strcmp (direntp->d_name , "..") == 0 ||
            strcmp (direntp->d_name ,".") == 0){
            continue;
        }
        string path;
        switch (direntp->d_type){
            case DT_DIR:
                //path = s + "/"+direntp->d_name;
                path = direntp->d_name;
                dirlist.push_back(path);
                break;
            case DT_REG: 
                //printf(" %s/%s \n",s.c_str(), direntp->d_name);
                filesize = getFileSize(s + "/" +direntp->d_name);
                totalSize += filesize;
                dirinfo->file.push_back(make_pair(direntp->d_name,filesize));
                break;
            case DT_UNKNOWN:printf("unknown %s \n", direntp->d_name); break; 
            default:break;
        }
    }
    closedir(dirp);
    for (auto a : dirlist){
        MyDir * p = enum_dir(s + "/" + a);
        if (p != NULL)
            dirinfo->sub_dir.push_back(p);
    }
    return dirinfo;
}
string getParentPath(CSTR& fileName){
    size_t begin = fileName.length();
    if (fileName[begin - 1] == '/'){
        begin--;
        begin--;
    } 
    size_t found = fileName.rfind("/",begin);
    if (found != string::npos)
        return fileName.substr(0,found);
    else
        return ".";
}
string getBaseFileName(CSTR& filename){
    size_t found = filename.rfind("/");
    if (found != string::npos)
        return filename.substr(found);
    else
        return filename;
}
void makeDir(CSTR& dirName){
    int size = getFileSize(dirName);
    if (size >= 0 )
        return ;
    string parent = getParentPath(dirName);
    if (parent != "")
        makeDir(parent);
    mkdir(dirName.c_str(), 0777);
}
void removeLast(string& s){
    int l = s.length();
    if (s[l-1] == '/')
        s.pop_back();
}
void showStatus(){
    time_t cur = time(NULL);
    cur = cur - beginTime ;
    double speed = finishedSize / cur;
    fprintf(stderr, 
            "\r F: %.2f \%!  S: %.2f MB  R: %.2f  E: %d ", 
            finishedSize * 100 / totalSize,
            speed / (1 << 20) , ( totalSize - finishedSize ) / speed,
            cur);
    fflush(stderr);

}
void copyDir(CSTR& srcDir, CSTR& dstDir){
    MyDir * p = enum_dir(srcDir.c_str());
    //p->dirname = srcDir;
    srcPath = getParentPath(srcDir);
    //cout << "srcPath : " << srcPath << endl;
    dstPath = dstDir;
    //cout << "dstPath:" << dstPath << endl;
    //p->p(".");
    beginTime = time(NULL);
    p->copy(".");
    p->del();
    delete p;
}
void copyFile(CSTR& srcFile,CSTR& dstFile){
    cout << "srcFile:"<<srcFile << endl
        << "dstFile:" << dstFile << endl;;
   int  size = getFileSize(srcFile);
   if (size == -1){
       printf(" srcFile %s error %d \n", srcFile.c_str(),errno);
       return ;
   }
 
   ifstream src(srcFile,ifstream::binary);
   if (!src.good()){
       printf("%s open error!",srcFile);
       return ;
   }
   int dsize = getFileSize(dstFile);
   if (dsize >= 0){
       printf("%s exists! \n",dstFile.c_str());
       return;
   }
   ofstream dst(dstFile,ofstream::binary);
   if (!dst.good()){
       string path = getParentPath(dstFile);
       if (path != ".") 
           makeDir(path);
       dst.open(dstFile,ofstream::binary);
       if (!dst.good()){
           cout << dstFile << " open error " << endl;
           return ;
       }
   }
  cout << srcFile << " size : " << size << endl;
   int rsize = size > BUFSIZE ? BUFSIZE : size;
   int count = 0;
   while(src){
       src.read(buff,rsize);
       finishedSize += rsize;
       count += rsize; 
       if (count >= BUFSIZE){
           showStatus();
           count = 0;
       }
       dst.write(buff,rsize);
       size -= rsize;
       if (size == 0)
           break;
       rsize = size > rsize ? rsize : size;
   }
   src.close();
   dst.close();
}

int main(int argc, char * argv[]){
    struct stat sb;
    char buf[255];
    getcwd(buf,255); 
    //string filename{buf};
    string srcFilename("");
    string dstFilename("");
    if (argc > 2 ){
        srcFilename = argv[1];
        dstFilename = argv[2]; 
    }else
        return 0;
    //printf("%s\n",filename.c_str());
    buff = new char [BUFSIZE];
    if (buff == NULL){
        printf("error new \n");
        return errno;
    }
    if (isDir(srcFilename)) {
        //cout << "dir copy " << endl;
        removeLast(srcFilename);
        removeLast(dstFilename);
        copyDir(srcFilename,dstFilename);
    }else{
        totalSize = getFileSize(srcFilename);
        if (dstFilename[dstFilename.length()-1] == '/')
            dstFilename = dstFilename + getBaseFileName(srcFilename);
        beginTime = time(NULL);
        copyFile(srcFilename, dstFilename);
    }
    printf("\n time : %d \n",time(NULL) - beginTime);
    delete buff;
    return 0;
}
