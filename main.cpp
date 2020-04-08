#include <string>
#include <cstring>
#include <list>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <signal.h>
#include <vector>
#include "opt.h"
using namespace std;
typedef const string CSTR;
class MyDir;
const int BUFSIZE =  1 << 25;
const int INBUFSIZE =  1 << 21;
const int OUTBUFSIZE =  1 << 21;
double totalSize = 0 ;
double  finishedSize = 0 ;
char *buff = NULL;
char *inbuff = NULL;
char *outbuff = NULL;
MyDir const *pMyDir = NULL;
void copyFile(CSTR& srcFile,CSTR& dstFile);
string srcPath;
string dstPath;
vector<CSTR*> g_str;
vector<ifstream *> g_ifstream;
vector<ofstream *> g_ofstream;
Opt * g_opt = NULL;
time_t beginTime = time(NULL);

long long getFileSize(CSTR& filename){
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
    list<MyDir const *> sub_dir;
    list<pair<string,int>> file;
    void copy(CSTR& dst) const{
        //g_str.push_back(&dst);
        string path;
        g_str.push_back(&path);
        if (dst != ".")
            path = dst + "/" + dirname;
        else
            path = dirname;
        for (auto const& a : file){
            copyFile(srcPath + "/" + path + "/" + a.first, 
                     dstPath + "/" + path + "/" + a.first);
        }
        for (auto const& a : sub_dir){
            a->copy(path);
        }
        g_str.pop_back();
        //g_str.pop_back();
    }
    void p(CSTR& path) const {
        cout <<  dirname << endl;
        for (auto a : file){
            cout << "  " << path +"/" +  dirname + "/" + a.first 
                << " [" << a.second << "]" 
                << endl;;
        }
            
        for (auto a : sub_dir)
            a->p(path + "/" + dirname);
    }

    ~MyDir(){
        for (auto a : sub_dir){
            //printf("delete %s \n" , a->dirname.c_str());
            delete a;
        }
    }
};
MyDir const * enum_dir(CSTR& s){
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
    long long  filesize;
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
        MyDir const * p = enum_dir(s + "/" + a);
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
    //fprintf(stderr,"finishedSize: %f , totalSize : %f \n",
    //      finishedSize, totalSize);
    fprintf(stderr, 
            "\r F: %.2f \%!  S: %.2f MB  R: %.0f s  E: %d s", 
            finishedSize * 100 / totalSize,
            speed / (1 << 20) , ( totalSize - finishedSize ) / speed,
            cur);
    fflush(stderr);

}
void copyDir(CSTR& srcDir, CSTR& dstDir){
     pMyDir = enum_dir(srcDir.c_str());
    //p->dirname = srcDir;
    srcPath = getParentPath(srcDir);
    //cout << "srcPath : " << srcPath << endl;
    dstPath = dstDir;
    //cout << "dstPath:" << dstPath << endl;
    //p->p(".");
    beginTime = time(NULL);
    pMyDir->copy(".");
    delete pMyDir;
}
void copyFile(CSTR& srcFile,CSTR& dstFile){
    cout << "srcFile:"<<srcFile << endl
        << "dstFile:" << dstFile << endl;;
   long long  size = getFileSize(srcFile);
   if (size == -1){
       printf(" srcFile %s error %d \n", srcFile.c_str(),errno);
       return ;
   }
 
   ifstream src(srcFile,ifstream::binary);
   if (!src.good()){
       printf("%s open error!",srcFile);
       return ;
   }
   long long  dsize = getFileSize(dstFile);
   if (dsize >= 0){
       if (!g_opt->has('r')){
           printf("%s exists! \n",dstFile.c_str());
           return;
       }
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
   g_ifstream.push_back(&src);
   g_ofstream.push_back(&dst);
   g_str.push_back(&srcFile);
   g_str.push_back(&dstFile);
  cout << srcFile << " size : " << size << endl;
   //dst << src.rdbuf();
   int rsize = size > BUFSIZE ? BUFSIZE : size;
   int count = 0;
   src.rdbuf()->pubsetbuf(inbuff,INBUFSIZE);
   dst.rdbuf()->pubsetbuf(outbuff,OUTBUFSIZE); 
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
   g_ifstream.pop_back();
   g_ofstream.pop_back();
   g_str.pop_back();
   g_str.pop_back();
}
void int_handler(int) {
    if (buff != NULL) delete [] buff;
    if (inbuff != NULL) delete [] inbuff;
    if (outbuff != NULL) delete [] outbuff;
    if (pMyDir != NULL ) delete pMyDir; 
    cout << "******" << endl;
    for (auto a : g_str){
        //cout << "delete " << *a << endl;
        a->~string();
    }
    for (auto a : g_ifstream){
        a->close();
    }
    for (auto a : g_ofstream) {
        a->close();
    }
    delete g_opt;
    cout << "******" << endl;
    //srcPath.~string();
    //dstPath.~string();
    /*
    g_str.~vector<CSTR*>();
    g_ifstream.~vector<ifstream*>();
    g_ofstream.~vector<ofstream*>();
    */
    exit(0);
}
void help(){
    cout 
        << "\n"
        <<"Copy recursive with status showing \n"
        <<"\n"
        << "\tmcp [Options] srcfile [ dstfile | dstdir ] \n"
        << "\tmcp [Options] srcdir dstdir \n"
        <<"\n"
        << "Options:\n"
        << "\t-r \t\t\t replace"
        << "\n"
        << endl;
}
int main(int argc, char * argv[]){
    sigset(SIGINT, int_handler);
    struct stat sb;
    char buf[255];
    getcwd(buf,255); 
    //string filename{buf};
    string srcFilename("");
    string dstFilename("");
    char optstring[] = "srp";
    Opt opt(argc,argv,optstring);
    g_opt = &opt;
    if (opt.argC() > 1 ){
        srcFilename = opt.argV(0);
        dstFilename = opt.argV(1);
    }else{
        help();
        return 0;
    }
    //printf("%s\n",filename.c_str());
    buff = new char [BUFSIZE];
    inbuff = new char [INBUFSIZE];
    outbuff = new char [OUTBUFSIZE];
    if (buff == NULL){
        printf("error new \n");
        return errno;
    }
    g_str.push_back(&srcFilename);
    g_str.push_back(&dstFilename);
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
    printf("\ntime : %d \n",time(NULL) - beginTime);
    delete [] outbuff;
    delete [] inbuff;
    delete [] buff;
    g_str.pop_back();
    g_str.pop_back();
    return 0;
}
