/*

M.A.P.S by ????
Creation date: 11/12/2026
FEU Institute of Technology

*/

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
// #define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

using namespace std;
namespace fs = filesystem;

// folder that holds the program itself (the only OS specific code besides
// homeDir)
fs::path exeDir() {
#ifdef _WIN32
  wchar_t buffer[MAX_PATH];
  DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  if (length > 0 && length < MAX_PATH)
    return fs::path(buffer).parent_path();
#elif defined(__APPLE__)
  char buffer[4096];
  uint32_t size = sizeof(buffer);
  if (_NSGetExecutablePath(buffer, &size) == 0)
    return fs::canonical(buffer).parent_path();
#else
  char buffer[4096];
  ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
  if (length > 0) {
    buffer[length] = '\0';
    return fs::path(buffer).parent_path();
  }
#endif
  return fs::current_path(); // fallback
}

// home folder of the current user
fs::path homeDir() {
  const char *home = getenv("HOME"); // linux, mac
  if (home == nullptr)
    home = getenv("USERPROFILE"); // windows
  if (home == nullptr)
    return fs::current_path();
  return fs::path(home);
}

string toLower(string text) {
  for (char &c : text)
    c = tolower((unsigned char)c);
  return text;
}

string toUpper(string text) {
  for (char &c : text)
    c = toupper((unsigned char)c);
  return text;
}

// removes spaces and line endings from both ends
string trim(const string &text) {
  size_t start = text.find_first_not_of(" \t\r\n");
  if (start == string::npos)
    return "";
  size_t end = text.find_last_not_of(" \t\r\n");
  return text.substr(start, end - start + 1);
}

// rejects empty names and characters that break folders
bool isValidName(const string &name) {
  if (name.empty() || name == "." || name == "..")
    return false;
  return name.find_first_of("<>:\"/\\|?*") == string::npos;
}

// replaces every {{name}} with the project name
string render(string text, const string &name) {
  const string key = "{{name}}";
  size_t pos = 0;
  while ((pos = text.find(key, pos)) != string::npos) {
    text.replace(pos, key.size(), name);
    pos += name.size();
  }
  return text;
}

string readFile(const fs::path &path) {
  ifstream in(path, ios::binary);
  stringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

void writeFile(const fs::path &path, const string &content) {
  ofstream out(path, ios::binary);
  out << content;
}

// settings stay in the user's home, program files may be read-only
fs::path configFile() { return homeDir() / ".maps_config"; }

// the config file holds one line: the head folder path
fs::path loadRoot() {
  ifstream in(configFile());
  string line;
  if (getline(in, line) && !line.empty())
    return fs::path(line);
  return homeDir() / "ProjectManager"; // default
}

// templates sit next to the program
fs::path templatesDir() { return exeDir() / "templates"; }

// turns cpp, c++, CPP... into the language folder name using aliases.txt
string languageFolder(const string &input) {
  string key = toLower(input);
  fs::path templates = templatesDir();

  ifstream aliases(templates / "aliases.txt");
  string line;
  while (getline(aliases, line)) {
    line = trim(line);
    size_t eq = line.find('=');
    if (line.empty() || line[0] == '#' || eq == string::npos)
      continue;
    if (toLower(trim(line.substr(0, eq))) == key)
      return trim(line.substr(eq + 1));
  }

  // no alias found: try an existing template folder, ignoring case
  if (fs::exists(templates))
    for (const auto &entry : fs::directory_iterator(templates))
      if (entry.is_directory() &&
          toLower(entry.path().filename().string()) == key)
        return entry.path().filename().string();
  return toUpper(input);
}

// copies a template folder, calls itself for every sub folder
void copyTemplate(const fs::path &from, const fs::path &to,
                  const string &name) {
  fs::create_directories(to);
  for (const auto &entry : fs::directory_iterator(from)) {
    fs::path target = to / render(entry.path().filename().string(), name);
    if (entry.is_directory())
      copyTemplate(entry.path(), target, name);
    else
      writeFile(target, render(readFile(entry.path()), name));
  }
}

void printHelp() {
  cout << "Usage:\n";
  cout << "  maps new <name> <lang> [type]   create a project (example: maps "
          "new "
          "calc cpp)\n";
  cout << "  maps list                       show all projects\n";
  cout << "  maps templates                  show available templates\n";
  cout << "  maps root                       show the head folder\n";
  cout << "  maps root <path>                change the head folder\n";
  cout << "  maps help                       show this message\n";
}

void newProject(const string &name, const string &lang, const string &type) {
  if (!isValidName(name) || !isValidName(lang) || !isValidName(type)) {
    cout << "Error: invalid name.\n";
    return;
  }
  string folder = languageFolder(lang);
  fs::path project = loadRoot() / folder / name;
  if (fs::exists(project)) {
    cout << "Error: project already exists.\n";
    return;
  }
  fs::path source = templatesDir() / folder / toLower(type);
  fs::create_directories(project);
  if (fs::is_directory(source))
    copyTemplate(source, project, name);
  else
    cout << "Note: no template at " << source.string()
         << ", made an empty project.\n";
  if (!fs::exists(project / "README.md"))
    writeFile(project / "README.md", "# " + name + "\n");
  cout << "Created " << project.string() << "\n";
}

// prints each folder and the folders inside it
void printFolders(const fs::path &root) {
  for (const auto &outer : fs::directory_iterator(root)) {
    if (!outer.is_directory())
      continue;
    cout << outer.path().filename().string() << "\n";
    for (const auto &inner : fs::directory_iterator(outer.path()))
      if (inner.is_directory())
        cout << "\t" << inner.path().filename().string() << "\n";
  }
}

void listProjects() {
  fs::path root = loadRoot();
  if (!fs::exists(root)) {
    cout << "No projects yet.\n";
    return;
  }
  printFolders(root);
}

void listTemplates() {
  fs::path dir = templatesDir();
  if (!fs::exists(dir)) {
    cout << "No templates folder found. It must sit here: " << dir.string()
         << "\n";
    return;
  }
  cout << "Templates folder: " << dir.string() << "\n";
  printFolders(dir);
}

void setRoot(const string &path) {
  fs::path root = fs::absolute(path);
  fs::create_directories(root);
  ofstream out(configFile());
  out << root.string() << "\n";
  cout << "Head folder set to " << root.string() << "\n";
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printHelp();
    return 0;
  }
  string command = argv[1];
  try {
    if (command == "new" && (argc == 4 || argc == 5))
      newProject(argv[2], argv[3], argc == 5 ? argv[4] : "default");
    else if (command == "list")
      listProjects();
    else if (command == "templates")
      listTemplates();
    else if (command == "root" && argc == 2)
      cout << loadRoot().string() << "\n";
    else if (command == "root" && argc == 3)
      setRoot(argv[2]);
    else if (command == "help")
      printHelp();
    else {
      cout << "Unknown command or wrong arguments.\n";
      printHelp();
      return 1;
    }
  } catch (const exception &e) {
    cout << "Error: " << e.what() << "\n";
    return 1;
  }
  return 0;
}