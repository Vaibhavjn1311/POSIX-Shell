#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <vector>
#include <filesystem>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <ctime>
#include <iomanip>
#include <dirent.h>
#include <cstring>
#include <sys/types.h>
#include <algorithm>
#include <sys/wait.h>
#include <fcntl.h>
using namespace std;
namespace fs = std::filesystem;
char pre_dir[1024];

string prompt_shell(int l){
    char path[PATH_MAX];
    string host_name;
    char* user = getlogin();
    char host[1024];
    int host_fd = gethostname(host,sizeof(host));
    if(user){
        string user_name = user;
    }
    else{
        perror("User Not found");
        exit(0);
    }
    if(host_fd==0){
        host_name = host;
    }

    if(getcwd(path,sizeof(path))!=NULL){
        return string(user)+"@"+host_name+":"+"~"+(path+l)+">"+" ";
    }
    else{
        perror("Path not Found");
        exit(0);
    }

}

void tokenize(char* command,char** args){
    char* token = strtok(command," \t\"");
    int i=0;
    while(token != NULL){
        args[i++] = token;
        token = strtok(NULL," \t\"");
    }
    args[i]=NULL;
    args[++i]=NULL;
}

void echo(char** args){

    int i = 1;
    while(args[i]!=NULL){
        cout<<args[i]<<" ";
        i++;
    }
    cout<<endl;

}

void pwd(int l){
    char curr_dir[PATH_MAX];
    getcwd(curr_dir, sizeof(curr_dir));
    cout<<"~"<<(curr_dir+l)<<endl;
    return ;
}

void cd(char** args,char c_path[]){
    
    char _path[1024];
    getcwd(_path,sizeof(_path));

    if(args[2]!=NULL){
        cerr<<"Invalid Argumnets."<<endl;
        return ;
    }
    else{
        if (args[1] == NULL){
            if(strcmp(c_path,_path)!=0){
                // cout<<"YES";
                chdir(c_path);
            }
            else{
                cout<<c_path<<endl;
            }
        }
            
        else if (strcmp(args[1],"~")==0){
            chdir(c_path);
        }

        else if (strcmp(args[1],"-")==0){
            chdir(pre_dir);
        }

        else if(strcmp(args[1],"..")==0 && strcmp(_path,c_path)==0){
            chdir(c_path);
        }

        else if (chdir(args[1]) != 0){
            perror("SHELL");
        }
        strncpy(pre_dir,_path,sizeof(pre_dir));
        return ;
    }

}

bool search_(char *file){
    char currentPath[1024];
    getcwd(currentPath,sizeof(currentPath));
    for (const auto& entry : fs::recursive_directory_iterator(currentPath)) {
            if (entry.path().filename() == file) {
                return true;
            }
        }
        return false;
}

void ls(char** args) {
    DIR* dir;
    struct stat mystat;
    struct passwd* user;
    struct group* group;
    char buffer[1024];
    vector<string> entries;
    int show_all = 0;
    int long_format = 0;

    int i = 1;
    while (args[i] != nullptr) {
        if (args[i][0] == '-') {
            for (int j = 1; args[i][j] != '\0'; ++j) {
                if (args[i][j] == 'a') show_all = 1;
                if (args[i][j] == 'l') long_format = 1;
            }
        } else {
            break;
        }
        i++;
    }
    string path = (args[i] == nullptr) ? "." : args[i];

    dir = opendir(path.c_str());
    if (dir == nullptr) {
        if(search_(args[1])){
            cout<<args[i]<<endl;
        }
        else{
            perror("opendir");
        }
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (show_all || entry->d_name[0] != '.') {
            entries.push_back(entry->d_name);
        }
    }
    closedir(dir);

    sort(entries.begin(), entries.end());

    if (long_format) {
        int file_count = entries.size();  
        cout << "total " << file_count << endl;
        
        int max_user_len = 0;
        int max_group_len = 0;
        int max_size_len = 0;

        for (const auto& entry_name : entries) {
            snprintf(buffer, sizeof(buffer), "%s/%s", path.c_str(), entry_name.c_str());
            if (stat(buffer, &mystat) == -1) {
                continue;
            }
            user = getpwuid(mystat.st_uid);
            group = getgrgid(mystat.st_gid);

            max_user_len = max(max_user_len, (int)strlen(user ? user->pw_name : "unknown"));
            max_group_len = max(max_group_len, (int)strlen(group ? group->gr_name : "unknown"));
            max_size_len = max(max_size_len, (int)to_string(mystat.st_size).length());
        }

        for (const auto& entry_name : entries) {
            snprintf(buffer, sizeof(buffer), "%s/%s", path.c_str(), entry_name.c_str());
            if (stat(buffer, &mystat) == -1) {
                continue;
            }

            cout << (S_ISDIR(mystat.st_mode) ? "d" : "-");
            cout << (mystat.st_mode & S_IRUSR ? "r" : "-");
            cout << (mystat.st_mode & S_IWUSR ? "w" : "-");
            cout << (mystat.st_mode & S_IXUSR ? "x" : "-");
            cout << (mystat.st_mode & S_IRGRP ? "r" : "-");
            cout << (mystat.st_mode & S_IWGRP ? "w" : "-");
            cout << (mystat.st_mode & S_IXGRP ? "x" : "-");
            cout << (mystat.st_mode & S_IROTH ? "r" : "-");
            cout << (mystat.st_mode & S_IWOTH ? "w" : "-");
            cout << (mystat.st_mode & S_IXOTH ? "x" : "-");

            cout << " " << setw(2) << mystat.st_nlink;

            user = getpwuid(mystat.st_uid);
            group = getgrgid(mystat.st_gid);
            cout << " " << setw(max_user_len) << left << (user ? user->pw_name : "unknown");
            cout << " " << setw(max_group_len) << left << (group ? group->gr_name : "unknown");

            cout << " " << setw(max_size_len) << right << mystat.st_size;

            time_t now;
            time(&now);
            int year = localtime(&now)->tm_year;
            tm* starttime = localtime(&mystat.st_ctime);
            char mod[13];
            if (starttime->tm_year == year) {
                strftime(mod, sizeof(mod), "%b %e %R", starttime);
            } else {
                strftime(mod, sizeof(mod), "%b %e %Y", starttime);
            }
            cout << " " << mod;

            cout << " " << entry_name << endl;
        }
    } else {
        for (const auto& entry_name : entries) {
            cout << entry_name << endl;
        }
    }
}



void pinfo(char** args) {
    char processPath[PATH_MAX];
    char statPath[PATH_MAX];
    char exePath[PATH_MAX];
    struct stat statBuf;
    FILE* statFile;
    pid_t pid = getpid();
    
    if (args[1]) {
        pid = atoi(args[1]);
    }
    
    snprintf(processPath, sizeof(processPath), "/proc/%d", pid);
    snprintf(statPath, sizeof(statPath), "%s/stat", processPath);
    
    statFile = fopen(statPath, "r");
    if (!statFile) {
        perror("Error reading stat file");
        return;
    }
    
    int pidFromStat;
    char comm[256];
    char state;
    
    fscanf(statFile, "%d %s %c", &pidFromStat, comm, &state);
    fclose(statFile);
    
    printf("pid: %d\n", pidFromStat);
    printf("Process Status: %c\n", state);
    
    snprintf(statPath, sizeof(statPath), "%s/statm", processPath);
    statFile = fopen(statPath, "r");
    if (!statFile) {
        perror("Error reading statm file");
        return;
    }
    
    long memorySize;
    fscanf(statFile, "%ld", &memorySize);
    fclose(statFile);
    
    long pageSize = sysconf(_SC_PAGESIZE);
    printf("Memory: %ld KB\n", memorySize * pageSize / 1024);
    
    snprintf(statPath, sizeof(statPath), "%s/exe", processPath);
    ssize_t len = readlink(statPath, exePath, sizeof(exePath) - 1);
    if (len == -1) {
        perror("Error reading symbolic link exe");
        return;
    }
    exePath[len] = '\0';
    
    string exePathStr(exePath);
    size_t homeDirPos = exePathStr.find(getenv("HOME"));
    if (homeDirPos == 0) {
        cout << "Executable Path: ~" << exePathStr.substr(homeDirPos + strlen(getenv("HOME"))).c_str() << endl;
    } else {
        cout << "Executable Path: " << exePathStr << endl;
    }
}

int redirect(char **args, int num)
{
    char in[100], out[100], app[100];
    int r = 0, pid, flag1 = 0, flag2 = 0, flag3 = 0;
    args[num] = nullptr;

    pid = fork();
    if (pid == 0) 
    {
        for (int i = 0; i < num; ++i)
        {
            if (strcmp(args[i], "<") == 0)
            {
                args[i] = nullptr;
                strcpy(in, args[i + 1]);
                flag1 = 1;
            }

            if (args[i] != nullptr)
            {
                if (strcmp(args[i], ">") == 0)
                {
                    args[i] = nullptr;
                    strcpy(out, args[i + 1]);
                    flag2 = 1;
                }
                else if (args[i][0] == '>' && args[i][1] == '>')
                {
                    args[i] = nullptr;
                    strcpy(app, args[i + 1]);
                    flag3 = 1;
                }
            }
        }

        if (flag1)
        {
            int fd = open(in, O_RDONLY);
            if (fd < 0)
            {
                std::cerr << "Could not open input file: " << in << std::endl;
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (flag2)
        {
            int fd;
            struct stat buf;
            if (stat(out, &buf) == 0) 
            {
                fd = open(out, O_WRONLY | O_TRUNC);
            }
            else
            {
                fd = creat(out, 0644);
            }
            if (fd < 0)
            {
                std::cerr << "Could not open/create output file: " << out << std::endl;
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        if (flag3)
        {
            int fd;
            struct stat buf;
            if (stat(app, &buf) == 0) 
            {
                fd = open(app, O_APPEND | O_WRONLY);
            }
            else // File does not exist
            {
                fd = creat(app, 0644);
            }
            if (fd < 0)
            {
                std::cerr << "Could not open/create append file: " << app << std::endl;
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        if (execvp(args[0], args) < 0)
        {
            std::cerr << args[0] << ": Command doesn't exist" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    else 
    {
        wait(nullptr);
    }

    return 1;
}


void pipehandler(char **args) {
    int filedes[2];
    int filedes2[2];

    int num_cmds = 0;

    char *command[256];

    pid_t pid;

    int err = -1, end = 0, i = 0, j = 0, k = 0, l = 0;

    while (args[l] != nullptr) {
        if (strcmp(args[l], "|") == 0) {
            num_cmds++;
        }
        l++;
    }
    num_cmds++;

    while (args[j] != nullptr && end != 1) {
        k = 0;

        while (strcmp(args[j], "|") != 0) {
            command[k] = args[j];
            j++;
            if (args[j] == nullptr) {
                end = 1;
                k++;
                break;
            }
            k++;
        }
        command[k] = nullptr;
        j++;

        if (i % 2 != 0) {
            pipe(filedes);
        } else {
            pipe(filedes2);
        }

        pid = fork();

        if (pid == -1) {
            if (i != num_cmds - 1) {
                if (i % 2 != 0) {
                    close(filedes[1]); 
                } else {
                    close(filedes2[1]);
                }
            }
            std::cerr << "Child process could not be created" << std::endl;
            return;
        }
        if (pid == 0) {
            if (i == 0) {
                dup2(filedes2[1], STDOUT_FILENO);
            } else if (i == num_cmds - 1) {
                if (num_cmds % 2 != 0) {
                    dup2(filedes[0], STDIN_FILENO);
                } else {
                    dup2(filedes2[0], STDIN_FILENO);
                }
            } else {
                if (i % 2 != 0) {
                    dup2(filedes2[0], STDIN_FILENO);
                    dup2(filedes[1], STDOUT_FILENO);
                } else {
                    dup2(filedes[0], STDIN_FILENO);
                    dup2(filedes2[1], STDOUT_FILENO);
                }
            }

            if (execvp(command[0], command) == err) {
                std::cerr << "Command not found: " << command[0] << std::endl;
                _exit(EXIT_FAILURE); 
            }
        }

        if (i == 0) {
            close(filedes2[1]);
        } else if (i == num_cmds - 1) {
            if (num_cmds % 2 != 0) {
                close(filedes[0]);
            } else {
                close(filedes2[0]);
            }
        } else {
            if (i % 2 != 0) {
                close(filedes2[0]);
                close(filedes[1]);
            } else {
                close(filedes[0]);
                close(filedes2[1]);
            }
        }

        waitpid(pid, nullptr, 0);

        i++;
    }
}

int launch(char** args, char c_path[], int l) {
    int i, background = 0, redirectflag = 0, piping = 0;

    for (i = 0; args[i] != NULL; i++) {
        if (args[i][0] == '>' || args[i][0] == '<') {
            redirectflag = 1;
        }
        if (args[i][0] == '|') {
            piping = 1;
        }
        if (args[i][0] == '&') {
            background = 1;
            args[i] = nullptr; 
        }
    }

    if (piping) {
        pipehandler(args);
    } else if (redirectflag) {
        redirect(args, i);
    } else {
        pid_t pid = fork();
        if (pid == 0) { 
            if (strcmp(args[0], "echo") == 0) {
                echo(args);
            } else if (strcmp(args[0], "pwd") == 0) {
                pwd(l);
            } else if (strcmp(args[0], "ls") == 0) {
                ls(args);
            } else if (strcmp(args[0], "pinfo") == 0) {
                pinfo(args);
            } else {
                execvp(args[0], args);
                perror("execvp");
                // exit(EXIT_FAILURE);
            }
        } else if (pid > 0) { // Parent process
            if (!background) {
                int status;
                waitpid(pid, &status, WUNTRACED);
            } else {
                cout << "Process " << pid << " running in background" << endl;
            }
        } else {
            perror("fork");
        }
    }

    return 1;
}



int main(){

    char c_path[PATH_MAX];
    getcwd(c_path,sizeof(c_path));
    int l = strlen(c_path);
    char command[1024];
    char* args[100];
    vector<string> manage_history;

    while(1){

        string dir = prompt_shell(l);

        cout<<dir;
        cin.getline(command,sizeof(command));

        if(manage_history.size()>=20){
            manage_history.erase(manage_history.begin());
        }
        manage_history.push_back(command);

        tokenize(command,args);


        if(strcmp(args[0],"cd")==0){
            cd(args,c_path);
            continue;
        }

        else if(strcmp(args[0],"search")==0){
            bool k = search_(args[1]);
            if(k){
                cout<<"File Found"<<endl;
            }
            else{
                cout<<"File not Found"<<endl;
            }
            continue;
        }

        else if(strcmp(args[0],"history")==0){
            if(manage_history.size()<10){
                for(int i=0;i<manage_history.size();i++){
                    cout<<manage_history[i]<<endl;
                }
                continue;
            }
            else{
                for(int i = manage_history.size()-10;i<manage_history.size();i++){
                    cout<<manage_history[i]<<endl;
                }
                continue;
            }
        }

        launch(args,c_path,l);
    }

    return 0;
}

