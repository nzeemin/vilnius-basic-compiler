
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#endif

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <assert.h>

typedef std::string string;

bool g_verbose = false;  // Verbose mode
int  g_failedtests = 0;

#ifdef _MSC_VER
HANDLE g_hConsole;
#define TEXTATTRIBUTES_TITLE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define TEXTATTRIBUTES_NORMAL (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define TEXTATTRIBUTES_WARNING (FOREGROUND_RED | FOREGROUND_GREEN)
#define TEXTATTRIBUTES_DIFF (FOREGROUND_RED | FOREGROUND_GREEN)
#define SetTextAttribute(ta) SetConsoleTextAttribute(g_hConsole, ta)
#define mkdir(dir) _mkdir(dir)
#else
#define SetTextAttribute(ta) {}
#define mkdir(dir) mkdir(dir, 0755)
#endif

#ifdef _MSC_VER
const char* TESTS_SUB_DIR = "tests";
const char* COMPILER_PATH = "Debug\\vibasc.exe";
const char* PATH_SEPARATOR = "\\";
#else
const char* TESTS_SUB_DIR = "tests/";
const char* COMPILER_PATH = "../../vibasc";
const char* PATH_SEPARATOR = "/";
#endif

#ifdef _MSC_VER
// Get all files by mask in the directory. Win32 specific method
void findallfiles_bymask(const string& dirname, const string& mask, std::vector<string>& result)
{
    string pattern(dirname);
    pattern.append("\\*").append(mask);
    WIN32_FIND_DATA data;
    HANDLE hFind;
    if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                result.push_back(data.cFileName);
        } while (FindNextFile(hFind, &data) != 0);
        FindClose(hFind);
    }
}
#else
// Get all files by mask in the directory. POSIX method
string findallfiles_bymask(const string& dirname, const string& mask, std::vector<string>& result)
{
    DIR* dirp = opendir(dirname.c_str());
    struct dirent* dp;
    while ((dp = readdir(dirp)) != nullptr)
    {
        if (dp->d_type & DT_DIR)
            continue;

        string filename(dp->d_name);
        if (filename.size() < mask.size() ||
            0 != filename.compare(filename.size() - mask.size(), mask.size(), mask))
            continue;

        result.push_back(filename);
    }
    closedir(dirp);
}
#endif

#ifdef _MSC_VER
// Get first file by mask in the directory. Win32 specific method
string findfile_bymask(const string& dirname, const string& mask)
{
    string pattern(dirname);
    pattern.append("\\*").append(mask);
    WIN32_FIND_DATA data;
    HANDLE hFind;
    if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                FindClose(hFind);
                return data.cFileName;
            }
        } while (FindNextFile(hFind, &data) != 0);
        FindClose(hFind);
    }
    return "";
}
#else
// Get first file by mask in the directory. POSIX method
string findfile_bymask(const string& dirname, const string& mask)
{
    DIR* dirp = opendir(dirname.c_str());
    struct dirent* dp;
    while ((dp = readdir(dirp)) != nullptr)
    {
        if (dp->d_type & DT_DIR)
            continue;

        string filename(dp->d_name);
        if (filename.size() < mask.size() ||
            0 != filename.compare(filename.size() - mask.size(), mask.size(), mask))
            continue;

        closedir(dirp);
        return filename;
    }
    closedir(dirp);
    return "";
}
#endif

void remove_file(const string& testdirpath, const string& filename)
{
    if (filename.empty())
        return;

    //std::cout << "Removing " << filename << std::endl;
    string fullpath = testdirpath + PATH_SEPARATOR + filename;
    if (0 != remove(fullpath.c_str()))
    {
        std::cout << "Failed to delete file " << filename << " : errno " << errno << std::endl;
        return;
    }
}

void remove_file_if_exists(const string& testdirpath, const string& filename)
{
    string filename2 = findfile_bymask(testdirpath, filename);
    if (filename2.empty())
        return;
    remove_file(testdirpath, filename);
}

void rename_file(const string& testdirpath, const string& filename, const string& filenamenew)
{
    if (filename.empty())
        return;

    //std::cout << "Renaming " << filename << " to " << filenamenew << std::endl;
    string fullpath = testdirpath + PATH_SEPARATOR + filename;
    string fullpathnew = testdirpath + PATH_SEPARATOR + filenamenew;
    if (0 != rename(fullpath.c_str(), fullpathnew.c_str()))
    {
        std::cout << "Failed to rename file " << filename << " to " << filenamenew << " : errno " << errno << std::endl;
        return;
    }
}

#ifdef _MSC_VER
void process_test_run(const string& workingdir, const string& modulename, const string& commandline, const string& outfilename)
{
    string outfilenamewithdir = workingdir + PATH_SEPARATOR + outfilename;
    SECURITY_ATTRIBUTES sa;  memset(&sa, 0, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    HANDLE hOutFile = ::CreateFile(
        outfilenamewithdir.c_str(),
        GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOutFile == INVALID_HANDLE_VALUE)
    {
        std::cout << "Failed to open log file: error " << ::GetLastError() << std::endl;
        return;
    }

    string fullcommand = modulename + " " + commandline;
    char command[512];
    strcpy(command, fullcommand.c_str());

    STARTUPINFO si;  memset(&si, 0, sizeof(si));  si.cb = sizeof(si);
    si.hStdOutput = hOutFile;
    si.hStdError = hOutFile;
    si.dwFlags |= STARTF_USESTDHANDLES;
    PROCESS_INFORMATION pi;  memset(&pi, 0, sizeof(pi));
    if (!::CreateProcess(
        modulename.c_str(), command, NULL, NULL, TRUE,
        CREATE_NO_WINDOW, NULL, workingdir.c_str(), &si, &pi))
    {
        std::cout << "Failed to run the test: error " << ::GetLastError() << std::endl;
        ::CloseHandle(hOutFile);
        return;
    }

    // Wait until child process exits
    ::WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles
    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);
    ::CloseHandle(hOutFile);
}
#else
void process_test_run(const string& workingdir, const string& modulename, const string& commandline, const string& outfilename)
{
    char bufcwd[PATH_MAX];
    getwd(bufcwd);
    chdir(workingdir.c_str());

    string fullcommand = modulename + " " + commandline + ">" + outfilename;
    //std::cout << "Running the test: " << fullcommand << std::endl;
    int result = std::system(fullcommand.c_str());
    if (result != 0)
    {
        std::cout << "Failed to run the test: result " << result << std::endl;
    }

    chdir(bufcwd);
}
#endif

void process_test(string testfilename)
{
    size_t dotpos = testfilename.find_last_of('.');
    assert(dotpos != string::npos);
    string testname = testfilename.substr(0, dotpos);

    SetTextAttribute(TEXTATTRIBUTES_NORMAL);
    std::cout << testname << std::endl;
    SetTextAttribute(TEXTATTRIBUTES_WARNING);
    
    // make test directory
    string testdirpath = string(TESTS_SUB_DIR) + PATH_SEPARATOR + testname + ".tmp";
    mkdir(testdirpath.c_str());

    string basicfilename = testname + ".ASC";
    remove_file_if_exists(testdirpath, basicfilename);
    string macfilename = testname + ".MAC";
    remove_file_if_exists(testdirpath, macfilename);
    string outfilename = testname + ".out";
    remove_file_if_exists(testdirpath, outfilename);

    // parse and split the test file
    char buffer[256];
    string testfilepath = string(TESTS_SUB_DIR) + PATH_SEPARATOR + testfilename;
    std::ifstream fs(testfilepath);
    //TODO: check for error
    fs.getline(buffer, sizeof(buffer));  // read compiler command line parameters line
    string compilerparams(buffer);
    fs.getline(buffer, sizeof(buffer));  // skip one line after the parameters
    // Copy the BASIC program lines
    string basicfilepath = testdirpath + PATH_SEPARATOR + testname + ".ASC";
    std::ofstream ofs(basicfilepath);
    //TODO: check for error
    while (!fs.eof())
    {
        fs.getline(buffer, sizeof(buffer));
        if (buffer[0] == '-')
            break;
        ofs.write(buffer, strlen(buffer));
        ofs.put('\n');
    }
    ofs.flush();
    ofs.close();
    // rest of the test file is expected errors
    std::vector<string> errorlines;
    while (!fs.eof())
    {
        fs.getline(buffer, sizeof(buffer));
        if (*buffer != 0)
            errorlines.push_back(buffer);
    }
    fs.close();

    // run the compiler
    string compilerpath(COMPILER_PATH);
    compilerparams.append(" ").append(testname + ".ASC");
    process_test_run(testdirpath, compilerpath, compilerparams, outfilename);

    std::vector<string> outlines;
    std::ifstream fsout(testdirpath + PATH_SEPARATOR + outfilename);
    //TODO: check for error
    while (!fsout.eof())
    {
        fsout.getline(buffer, sizeof(buffer));
        if (*buffer != 0)
            outlines.push_back(buffer);
    }
    fsout.close();

    bool outlinesdifferent = false;
    if (errorlines.size() != outlines.size())
    {
        outlinesdifferent = true;
    }
    else
    {
        for (size_t i = 0; i < errorlines.size(); i++)
        {
            if (errorlines[i] != outlines[i])
            {
                outlinesdifferent = true;
                break;
            }
        }
    }
    if (outlinesdifferent)
    {
        std::cout << "  FAILED: Out lines are different." << std::endl;
        g_failedtests++;
        return;
    }

    if (!errorlines.empty())
        return;  // have errors, no need to check anything else

    // check if we have .MAC file
    string macfilename2 = findfile_bymask(testdirpath, macfilename);
    if (macfilename2.empty())
    {
        std::cout << "  FAILED: .MAC file not found." << std::endl;
        g_failedtests++;
        return;
    }

    // read .MAC file and check if we have TODOs there
    bool machastodos = false;
    std::ifstream fsmac(testdirpath + PATH_SEPARATOR + macfilename);
    //TODO: check for error
    while (!fsmac.eof())
    {
        fsmac.getline(buffer, sizeof(buffer));
        string line(buffer);
        if (line.find("TODO") != string::npos)
        {
            machastodos = true;
            break;
        }
    }
    fsmac.close();
    if (machastodos)
    {
        std::cout << "  FAILED: .MAC file contains TODOs." << std::endl;
        g_failedtests++;
        return;
    }
}

void parse_commandline(int argc, char* argv[])
{
    for (int argi = 1; argi < argc; argi++)
    {
        const char* arg = argv[argi];
        if (arg[0] == '\\' || arg[0] == '-')
        {
            string option = arg + 1;
            if (option == "v" || option == "verbose")
                g_verbose = true;
            else
            {
                std::cout << "Unknown option: " << option << std::endl;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    // Show title message
#ifdef _MSC_VER
    g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
    SetTextAttribute(TEXTATTRIBUTES_TITLE);
    std::cout << "TestRunner utility for VIBASC project" << std::endl;
    SetTextAttribute(TEXTATTRIBUTES_NORMAL);

    // Parse command line
    parse_commandline(argc, argv);

    // Collect list of test cases
    string testpathwithmask = string(TESTS_SUB_DIR);
    std::vector<string> testfilenames;
    findallfiles_bymask(TESTS_SUB_DIR, ".test", testfilenames);
    // Run all the test cases
    for (string& testfilename : testfilenames)
    {
        process_test(testfilename);
    }

    SetTextAttribute(TEXTATTRIBUTES_TITLE);
    std::cout << "TOTAL tests executed: " << testfilenames.size();
    if (g_failedtests > 0)
        std::cout << ", failed: " << g_failedtests;
    std::cout << std::endl;
    SetTextAttribute(TEXTATTRIBUTES_NORMAL);

    return 0;
}
