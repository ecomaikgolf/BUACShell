/*
 * Unofficial terminal application for
 * the University of Alicante library
 * written in C++
 *
 * Developed by:
 *  - Ignacio Encinas Rubio [1]
 *  - Ernesto Martínez García [1]
 *
 *  [1] Computer Science, University of Alicante Polytechnic School
 *
 * Features:
 *  - Lightweight (~70K)
 *  - Fast (C++)
 *  - Easy build from source (make)
 *  - Non bloated with dependencies (libcurl)
 *  - Scriptable (cron)
 *  - Autologin protected with system permissions
 *  - Open Source!
 * 
 *  https://github.com/ecomaikgolf/BUACShell
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <fstream>
#include <string.h>
#include <termios.h>
#include <unistd.h>

using namespace std;

/* Constants for connecting to the BUA */
static const string baseURL = "http://gaudi.ua.es";
static const string loginEnd = "/uhtbin/cgisirsi/0/0/0/29/124/X/3";
static const string userAgent = "UACloud Shell Interface";

/* Maximum size of _user, go to setup_files() */
#define MAXUSER 64

/* Constants for user storage */
static const string DEFAULT_FILEPATH = "/.config/uacshell/";
static const string RESERVATION_FILE = "reserva.txt";
static const string USER_FILE        = "user.toml";

/* Obtained at execution time */
static string HomeDir; /* "/home/" + USER */
static string ReservationFile; /* HomeDir + DEFAULT_FILEPATH + RESERVATION_FILE  */
static string UserFile; /* HomeDir + DEFAULT_FILEPATH + USER_FILE */

/* Nº of characters to skip during the renewed books HTML parsing */
static const unsigned int TRESHOLD = 40000;

/* Colours definition */
#define GREEN(x)  "\033[1;32m"     \
                    << x           \
                    << " \033[0m"  \

#define CYAN(x)   "\033[1;36m"     \
                    << x           \
                    << " \033[0m"  \

#define RED(x)    "\033[1;31m"     \
                    << x           \
                    << " \033[0m"  \

#define YELLOW(x) "\033[1;33m"     \
                    << x           \
                    << " \033[0m"  \

/* User file identifier */
#define bua "[bua]"

#define NUMBER_OF_ARGS 2

/* Valid commands */
static const char* PARAMETER_LIST[] = {
    "list",
    "renew",
    NULL
};

enum Args {
    List  = 0,
    Renew = 1,
    Error = 2,
};

/* Usage text */
static void exampleUsage() {
    cout << "Usage: bua COMMAND\n" 
         << "\n Command list: \n"
         << "  renew [-f]  Renews all non-reserved books\n"
         << "  list        Lists reserved books from local cache\n"
         << "\n Parameter list: \n"
         << "  [-f] Force credential input\n";
}

/* Argument manager */
static Args parseArguments(int argc, char *argv[]) {
    if(argc == 1) {
        exampleUsage();
        return Args::Error;
    } else if(argc > 1) {
        for(int i = 0; i < NUMBER_OF_ARGS; i++)
            if(!strcmp(argv[1], PARAMETER_LIST[i]))
                return (Args)i;
        exampleUsage();
        return Args::Error;
    }
    return Args::Error;
}

/* Accent/ special character values */
static const int special_char_int[] = {
    -31, -23, -19, -13, -6, -15, -47
};
static const string special_char[] = {
    "á", "é", "í", "ó", "ú", "ñ", "Ñ"
};

#define NUM_SPECIAL_CHAR 7

/* Accent managing */
string& parseString(string& str) {
    for(unsigned i = 0; i < str.size(); i++) {
        if((int) str[i] < 0) {
            for(int j = 0; j < NUM_SPECIAL_CHAR; j++)
                if((int) str[i] == special_char_int[j])
                    str.replace(i, 1, special_char[j]);
        }
    }
    return str;
} 

/* Book data structure */
struct Book {
    public: 
        bool renewed;
        string name;
        string author;
        string details;
        string returnTime; 
    public:
        Book(string &name, string &author, string &details, const string &returnTime) {
            renewed = false;
            this->name = parseString(name);
            this->author = parseString(author);
            this->details = parseString(details);
            this->returnTime = returnTime;
        }
        Book() {}
};

/* Book print operator */
ostream& operator<<(ostream& os, const Book& b) {
    if(b.renewed == true) {
        os << CYAN(b.name) << '\n'
           << b.author << '\n'
           << b.details << '\n'
           << GREEN("Renewed until")
           << GREEN(b.returnTime) << "\n\n";
    } else {
        os << RED(b.name)  << '\n'
           << b.author << '\n'
           << b.details << '\n'
           << RED("The book has been reserved")
           << RED(b.returnTime) << "\n\n";
    }
    return os;
}

string& cleanStr(string &str, const char delimiter) {
    stringstream sstream = stringstream(str);
    string aux;
    str.clear();
    str.reserve(50);

    sstream >> str;
    getline(sstream, aux, delimiter);
    str += aux;
    return str;
}

/* Parses renewed books from an HTML */
vector<Book> parseBooks(const string &str) {
    vector<Book> books;
    string action;

    string kBOOK = "Pr";
    kBOOK += -23; // é 
    kBOOK += "stamo renovado";
    const char delimiter = '<';
    stringstream tmp(str);
    string aux;

    while(getline(tmp, aux)) {
        if(aux.find(kBOOK) != string::npos) {
            Book book;
            book.renewed = true;
            for(unsigned i = 0; i < 4; i++)
                getline(tmp, aux);

            book.name = parseString(cleanStr(aux, delimiter));

            getline(tmp,aux);
            book.author = parseString(cleanStr(aux, delimiter));

            getline(tmp,aux);
            book.details = parseString(cleanStr(aux, delimiter));

            getline(tmp,aux);
            getline(tmp,aux);
            stringstream sstream(aux.substr(aux.find('>')+1, aux.size()-1));
            getline(sstream, book.returnTime, delimiter);

            books.push_back(book);
        } else if(aux.find("El documento tiene reserva") != string::npos) {
            Book book;
            book.renewed = false;
            for(unsigned i = 0; i < 4; i++)
                getline(tmp, aux);

            book.name = cleanStr(aux, delimiter);

            getline(tmp,aux);
            book.author = cleanStr(aux, delimiter);

            getline(tmp,aux);
            book.details = cleanStr(aux, delimiter);

            books.push_back(book);
        }
    }
    cout << endl;
    for(unsigned i = 0; i < books.size(); i++)
        cout << books[i];

    return books;
}

/* Libcurl HTML output handler */
string tempBuffer;
size_t curl_output(void *ptr, size_t size, size_t nmemb, void *stream){
    tempBuffer.append((char*)ptr, size*nmemb);
    return size*nmemb;
}

/* Obtains temporary access token */
string parseAccessForm(const string &str) {
    string action;

    const string kAccessForm = "name=\"accessform\"";
    const string wanted_field = "action=";
    stringstream tmp(str);
    string aux;

    while(getline(tmp, aux)) {
        if(aux.find(kAccessForm) != string::npos) {
            size_t stringPos = aux.find(wanted_field);
            if(stringPos == string::npos) {
                cout << RED("[!] The accessform could not be extracted") << endl;
                return action; 
            }
            stringstream actiontmp(aux.substr(stringPos + wanted_field.size() + 1 , aux.size()-1));
            getline(actiontmp, action, '"');
            return action;
        }
    }
    return action;
}

/* Update cache file */
bool saveReservation(vector<Book> books, string outputFile = ReservationFile, const ios_base::openmode flags = ofstream::out) {
    ofstream ofs;
    ofs.open(outputFile, flags);

    if(!ofs.is_open())
        return false;

    for(unsigned i = 0; i < books.size(); i++) {
        if(books[i].renewed == false) {
            cout << YELLOW("[?] Due to the library backend, we cannot retrieve non-renewed book dates") << endl;
            books[i].returnTime = "Unknown, check biblioteca.ua.es";
        }
        ofs << books[i];
    }
    return true;
}

/* Disallow character printing while stdin */
void hideSTDIN() {
    termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

/* Allow character printing while stdin */
void showSTDIN() {
    termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

/* Credentials input */
bool getCredentials(string &dni, string &nip) {
    cout << "[>] Introduce your DNI: ";
    hideSTDIN();
    cin >> dni;
    cout << '\n';
    cin.clear();

    if(isalpha(dni[dni.size() - 1]))
        dni.resize(dni.size() - 1);

    if(dni.size() == 8 && dni[0] != 0)
        dni = std::string("0") + dni;

    if(dni.size() != 9 )
        return false;

    for(auto it = dni.begin() ; it < dni.end(); it++) {
        if(!isdigit(*it))
            return false;
    }
    cout << "[>] Introduce your BUA NIP: ";
    cin >> nip;
    cin.clear();

    showSTDIN();

    cout << endl;

    return true;
}

/* Checks if a file exists */
inline bool exists_file(const string& filepath) {
    return (access(filepath.c_str(), F_OK) != -1);
}

bool obtainUserInfo(uid_t &realUID, uid_t &effectiveUID, uid_t &savedUID) {
    /* Obtaining UID information from system */
    if(getresuid(&realUID, &effectiveUID, &savedUID) < 0){
        cout << RED("[!] Fatal error. We cannot obtain UID information from system") << endl;
        return false;
    }
    return true;
}

bool obtainGroupInfo(gid_t &realGID, gid_t effectiveGID, gid_t savedGID) {
    /* Obtaining GID information from system */
    if(getresgid(&realGID, &effectiveGID, &savedGID) < 0){
        cout << RED("[!] Fatal error. We cannot obtain GID information from system") << endl;
        return false;
    }
    return true;
}

bool privilegesEscalation() {
    /* Privileges escalation to 0 (root) */
    if(setegid((gid_t)0) < 0 || seteuid((uid_t)0) < 0){
        cout << RED("[!] Fatal error. Check if you compiled with sudo privileges") << endl;
        return false;
    }
    return true;
}

bool dropPrivileges(uid_t realUID, uid_t effectiveUID, uid_t savedUID,
                    gid_t realGID, gid_t effectiveGID, gid_t savedGID) {
    /* Dropping privileges */
    if(setresgid(realGID, effectiveGID, savedGID) < 0){
        cout << RED("[!] Fatal error. We cannot restore GID") << endl;
        return false;
    }
    if(setresuid(realUID, effectiveUID, savedUID) < 0){
        cout << RED("[!] Fatal error. We cannot restore UID") << endl;
        return false;
    }
    return true;
}

int readData(string &dni, string &nip){
    /* UID and GID variable information */
    uid_t realUID = 0, effectiveUID = 0, savedUID = 0;
    gid_t realGID = 0, effectiveGID = 0, savedGID = 0;

    if(!obtainUserInfo(realUID, effectiveUID, savedUID) || !obtainGroupInfo(realGID, effectiveGID, savedGID)) {
        return -1;
    }

    if(!privilegesEscalation()) {
        return -1;
    }
    
    /* Reading privileged file. Hardcoded for safety */
    ifstream _file = ifstream(UserFile);

    string tmp;
    while(_file >> tmp) {
        if(tmp == bua) {
            getline(_file, tmp, '=');
            _file >> dni;

            getline(_file, tmp, '=');
            _file >> nip;

            if(dni.size() != 9) {
                cout << RED("[!] Check your DNI in ~/.config/uacshell/user.toml") << endl;
                return -1;
            }
        }
    }

    if(!dropPrivileges(realUID, effectiveUID, savedUID, realGID, effectiveGID, savedGID))
        return -1;

    if(dni == "" || nip == "") {
        cout << YELLOW("[?] Warning: Missing local user data, automatic login disabled.") << endl;
        return -1;
    }

    return 0;
}

bool setup_files() {
    char _user[MAXUSER];
    if(getlogin_r(_user, MAXUSER) || strlen(_user) > MAXUSER - 1) {
        cout << RED("[!] Could not read the user executing the program or its name is too big");
        return false;
    }
    HomeDir = "/home/";
    HomeDir += _user;

    string _local = HomeDir + DEFAULT_FILEPATH;
    UserFile = _local + USER_FILE;
    ReservationFile = _local + RESERVATION_FILE;
    return true;
}

int main(int argc, char *argv[]){
    bool ask_from_terminal = false;
    bool dont_save_reservation = false;

    if(!setup_files()) {
        return -1;
    }

    switch (parseArguments(argc, argv)) {
    case List:
        if(exists_file(ReservationFile)){
            cout << endl;
            execlp("cat", "cat", ReservationFile.c_str(), (char*)NULL);
        }

        cout << RED("[!] Local cache") << RED(ReservationFile) << RED("is missing") << endl;
        return -1;
    case Renew: 
        if(argc > 2 && !strcmp(argv[2], "-f")) {
            ask_from_terminal = true;
            /* Does not save the reservation cache if you renew with -f */
            dont_save_reservation = true;
        }
        break;
    case Error:
        return Args::Error;
    default:
        cout << RED("[!] Error: You should not have arrived here. Please inform of this bug to the developers\n");
        return -1;
    }

    string dni, nip;

    /* We try to read the user.toml file. If it isnt found or the data has wrong format, ask user input */
    int readDataStatus = -1;
    if(!ask_from_terminal)
        readDataStatus = readData(dni, nip);

    if(readDataStatus < 0 && getCredentials(dni, nip) == false) {
        cout << RED("[!] Invalid data, try again.\n");
        return -1;
    }

    CURL *curlHandler;
    curlHandler = curl_easy_init();
    
    curl_easy_setopt(curlHandler, CURLOPT_URL, (baseURL + loginEnd).c_str());
    curl_easy_setopt(curlHandler, CURLOPT_USERAGENT, userAgent.c_str());
    curl_easy_setopt(curlHandler, CURLOPT_WRITEFUNCTION, curl_output);
    curl_easy_perform(curlHandler);

    string tokenURL = parseAccessForm(tempBuffer);

    string parameters = "user_id=" + dni;
    parameters += "&password=" + nip;

    curl_easy_setopt(curlHandler, CURLOPT_URL, (baseURL + tokenURL).c_str());
    curl_easy_setopt(curlHandler, CURLOPT_USERAGENT, userAgent.c_str());
    curl_easy_setopt(curlHandler, CURLOPT_POSTFIELDS, parameters.c_str());
    curl_easy_setopt(curlHandler, CURLOPT_WRITEFUNCTION, curl_output);
    curl_easy_perform(curlHandler);

    std::size_t pos = tempBuffer.find("<p><strong>Acceso inv", TRESHOLD);
    if(pos != string::npos){
        cout << RED("[!] Incorrect DNI/NIP ");
        return 1;
    }

    parameters = "user_id=" + dni;
    parameters += "&selection_type=all";

    tokenURL.back() = '3';

    tempBuffer.clear();
    curl_easy_setopt(curlHandler, CURLOPT_URL, (baseURL + tokenURL).c_str());
    curl_easy_setopt(curlHandler, CURLOPT_USERAGENT, userAgent.c_str());
    curl_easy_setopt(curlHandler, CURLOPT_POSTFIELDS, parameters.c_str());
    curl_easy_setopt(curlHandler, CURLOPT_WRITEFUNCTION, curl_output);
    curl_easy_perform(curlHandler);

    vector<Book> books = parseBooks(tempBuffer);

    if(books.size() == 0) {
        cout << YELLOW("[*] You don't have any pending loan ") << '\n';
    } else if(!dont_save_reservation && saveReservation(books) == false) {
        cout << YELLOW("[*] We could not update the cache ") << ReservationFile << '\n';
    }

    if(readDataStatus < 0) {
        char response = 'N';
        cout << "[>] Store your credentials locally and enable automatic login? (y/n) ";
        cin >> response;
        cin.clear();
        
        if(response != 'Y' && response != 'y')
            return 0;

        if(!exists_file(UserFile)) {
            cout << RED(UserFile + " does not exist, create it by executing setup.sh") << endl;
            return -1;
        }
        
        ofstream of(UserFile);
        if(!of.is_open())
            return -1;

        of << "[bua]\ndni = " + dni + "\nnip = " + nip + '\n';
    }

    int counter = 0;
    for(unsigned i = 0; i < books.size(); i++)
        if(books[i].renewed == false)
            counter++;

    /* In a non-failure case, returns the number of non-renewed books */
    return counter;
}
