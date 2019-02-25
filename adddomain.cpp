#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
using namespace std;

void help();
void checkDir();
void createConfig(string path, string domain, string type);
string createDir(string dir);
bool domainExist(string domain);

int main(int argc, char* argv[])
{
    map<char, string> options;

    if (getenv("SUDO_USER") == NULL) {
        cout << "Use sudo to make this operation" << endl;
        return 403;
    }

    while (*++argv) {
        if ((*argv)[0] == '-') {
            switch ((*argv)[1]) {
                case 'h':
                // Help info about using this app
                    help();
                    return 0;
                    break;
                case 'c':
                // creating new domain
                    options.insert(pair<char, string>((*argv)[1], "yes"));
                    break;
                case '-':
                    if ((*argv)[2] == 'd' || (*argv)[2] == 't') {
                        options.insert(pair<char, string>((*argv)[2], string((*argv)).substr(3, string((*argv)).length())));
                        break;
                    }
                default:
                    cout << "Unknown key \"" << (*argv) << "\"" << endl;
                    break;
            }
        }
    }

    if (options.find('c') != options.end()) {
        if (options.find('d') != options.end()) {
            if (options.find('t') != options.end() && (options['t'] == "vue" || options['t'] == "laravel")) {
                if (!domainExist(options['d'])) {
                    checkDir();
                    string path = createDir(options['d']);
                    createConfig(path, options['d'], options['t']);
                } else {
                    cout << "This domain is laready exists" << endl;
                }
            } else {
                cout << "Use --t to select correct content type of your domain" << endl;
                cout << "Use -h for mote info" << endl;
            }
        } else {
            cout << "Use -d- to select name of new domain" << endl;
            cout << "Use -h for mote info" << endl;
        }
    }

    return 0;
}

void help()
{
    cout << "----Domain Creator Help Info----" << endl;
    cout << "Available keys: -h -c, Available arguments --d --t" << endl;
    cout << "--------------Keys--------------" << endl;
    cout << "-h - Help info" << endl;
    cout << "-c - Create new Domain" << endl;
    cout << "-----------Arguments------------" << endl;
    cout << "--d domainname. Example: --ddev01" << endl;
    cout << "will create dev01." << getenv("SUDO_USER") << ".buhojmedved.ru" << endl;
    cout << "--t type of content - laravel/vue, default laravel" << endl;
    cout << "using for generating configs of nginx" << endl;
    cout << "--------------End--------------" << endl;
}

void checkDir()
{
    register uid_t owner = atoi(getenv("SUDO_UID"));
	register gid_t group = atoi(getenv("SUDO_GID"));
    ifstream www;
    string dir_path = "/home/" + string(getenv("SUDO_USER")) + "/www";
    www.open(dir_path);
    if (!www.is_open()) {  
        mkdir(dir_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        chown(dir_path.c_str(), owner, group);
    }
}

string createDir(string dir)
{
    register uid_t owner = atoi(getenv("SUDO_UID"));
	register gid_t group = atoi(getenv("SUDO_GID"));
    string dir_path = "/home/" + string(getenv("SUDO_USER")) + "/www/" + dir + "." + string(getenv("SUDO_USER"));
    mkdir(dir_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    chown(dir_path.c_str(), owner, group);

    return dir_path;
}

void createConfig(string path, string domain, string type)
{
    ifstream preset;
    ofstream config;
    string root, preset_path, try_files;
    char buffer[100];

    string uri = "server_name " + domain + "." + string(getenv("SUDO_USER")) + ".buhojmedved.ru;";
    string config_path = "/etc/nginx/sites-available/" + domain + "." + string(getenv("SUDO_USER"));
    string fpm = "fastcgi_pass unix:/var/run/php/php7.2-fpm-" + string(getenv("SUDO_USER")) + ".sock;";
    if (type == "vue") {
        root = "root " + path + ";";
        preset_path = "/etc/nginx/sites-available/presets/vue";
        try_files = "";
    } else if (type == "laravel") {
        root = "root " + path + "/public;";
        preset_path = "/etc/nginx/sites-available/presets/laravel";
        try_files = "";
    }
    preset.open(preset_path.c_str());
    config.open(config_path.c_str());

    while (preset.getline(buffer, sizeof(buffer))) {
        string line(buffer);
		if (line == "#root#")
		{
			config << "\t" << root << endl;
		} else if(line == "#server_name#") {
			config << "\t" << uri << endl;
		} else if (line == "#fpm_socket#") {
			config << "\t\t" << fpm << endl;
		} else {
			config << line << endl;
		}
    }


    preset.close();
    config.close();

    string available = "/etc/nginx/sites-available/" + domain + "." + string(getenv("SUDO_USER"));
	string enabled = "/etc/nginx/sites-enabled/" + domain + "." + string(getenv("SUDO_USER"));
    symlink(available.c_str(), enabled.c_str());

    system("nginx -s reload");

    cout << "Domain \"" + domain + ".buhojmedved.ru\" has been created" << endl;
    cout << "Work directory is \"" + path + "\"" << endl; 
    cout << "Have a nice work" << endl;
}

bool domainExist(string domain)
{
    ifstream config;
    string dir_path = "/etc/nginx/sites-available/" + domain + "." + string(getenv("SUDO_USER"));
    config.open(dir_path);
    if (config.is_open()) {  
        return true;
    }

    return false;
}
