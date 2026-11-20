/*

M.A.P.S by Taurus Software
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
// check if NOMINMAX should be used
#ifndef NOMINMAX
#define NOMINMAX
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

using namespace std;
namespace fs = filesystem;

// a project folder is only shown by "list" if it holds this file
// this ".maps" marker is the tracker whether a project is created by this
// program
const string MARKER = ".maps";

// folder that holds the program itself (the only OS specific code besides
// homeDir

// old platform checker
/*
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
*/

fs::path exeDir() {
  wchar_t buffer[MAX_PATH];
  DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  if (length > 0 && length < MAX_PATH)
    return fs::path(buffer).parent_path();
  return fs::current_path(); // fallback
}

// home folder of the current user
fs::path homeDir() {

  // not removing directory check for linux and mac for fuutre purposes (same as
  // above)
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

// asks a yes/no question, only "y" or "yes" counts as yes
bool confirm(const string &question) {
  cout << question << " (y/n): ";
  string answer;
  getline(cin, answer);
  answer = toLower(trim(answer));
  return answer == "y" || answer == "yes";
}

// settings stay in the user's home, program files may be read-only
fs::path configFile() { return homeDir() / ".maps_config"; }

// the config file holds one line: the head folder path
fs::path loadRoot() {
  ifstream in(configFile());
  string line;
  if (getline(in, line) && !line.empty())
    return fs::path(line);
  return homeDir() / "MAPS"; // default
}

// templates now located permanently on %APPDATA%/MAPS/...
fs::path templatesDir() {
  const char *appData = getenv("APPDATA");
  if (appData == nullptr)
    return homeDir() / ".maps" / "templates";
  return fs::path(appData) / "MAPS" / "templates";
}

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

void showInfo() {
  fs::path root = loadRoot();
  fs::path templates = templatesDir();
  fs::path config = configFile();
  cout << "Head folder : " << root.string()
       << (fs::exists(root) ? "" : "  (not made yet)") << "\n";
  cout << "Templates   : " << templates.string()
       << (fs::exists(templates) ? "" : "  (missing)") << "\n";
  cout << "Settings    : " << config.string()
       << (fs::exists(config) ? "" : "  (not set, default head folder in use)")
       << "\n";
  cout << "Marker file : " << MARKER << " (inside each project)\n";
}

void printHelp() {
  cout << "M.A.P.S - Modular Architecture Project Scaffolder" << endl;
  cout << "Current Head folder: " << loadRoot().string() << "\n" << endl;
  cout << "Usage:" << endl;
  cout << "\tmaps new <project_name> <lang> [type]   create a project "
          "(example: maps "
          "new calc cpp)"
       << endl;
  cout << "\tmaps import <path> <lang>       move a project folder into the "
          "head folder"
       << endl;
  cout << "\tmaps list                       show all projects" << endl;
  cout << "\tmaps templates                  show available templates" << endl;
  cout << "\tmaps root                       show the head folder" << endl;
  cout << "\tmaps root <path>                change the head folder (full "
          "path, folder must exist)"
       << endl;
  cout << "\tmaps root reset                 go back to the default head "
          "folder"
       << endl;
  cout << "\tmaps help                       show this help message" << endl;
  cout << "\tmaps info                       show important file/folder "
          "locations"
       << endl;
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
  writeFile(project / MARKER, "created by MAPS\n");
  cout << "Created " << project.string() << "\n";
}

// true if path is the same folder as base, or inside it
bool isInside(const fs::path &path, const fs::path &base) {
  if (base.empty() || !fs::exists(base))
    return false;
  for (fs::path p = path;; p = p.parent_path()) {
    if (fs::exists(p) && fs::equivalent(p, base))
      return true;
    if (p == p.parent_path())
      return false;
  }
}

// returns why a folder must not be moved, or an empty text if it is fine
string protectedReason(const fs::path &source, const fs::path &root) {
  // drive roots and top level folders (C:\Windows, C:\Users, ...)
  if (source.parent_path() == source.root_path())
    return "it is a drive root or a top level folder";

  // system and program folders, and everything inside them
  const char *systemVars[] = {"SystemRoot",        "ProgramFiles",
                              "ProgramFiles(x86)", "ProgramData",
                              "APPDATA",           "LOCALAPPDATA"};
  for (const char *name : systemVars) {
    const char *value = getenv(name);
    if (value != nullptr && isInside(source, fs::path(value)))
      return "it is a system or program folder";
  }

  // the folder that holds the running program
  if (isInside(exeDir(), source))
    return "it holds the program";

  // the home folder and its own folders (Documents, Desktop, ...)
  fs::path home = homeDir();
  fs::path parent = source.parent_path();
  if (fs::exists(home) && fs::equivalent(source, home))
    return "it is your home folder";
  bool inHome =
      fs::exists(home) && fs::exists(parent) && fs::equivalent(parent, home);
  bool inOneDrive =
      toLower(parent.filename().string()).rfind("onedrive", 0) == 0;
  string name = toLower(source.filename().string());
  const char *userFolders[] = {
      "documents", "desktop",  "downloads", "pictures",    "music",
      "videos",    "onedrive", "appdata",   "saved games", "favorites",
      "links",     "contacts", "searches",  "3d objects"};
  if (inHome || inOneDrive)
    for (const char *folder : userFolders)
      if (name == folder)
        return "it is a user folder such as Documents or Desktop";
  if (inHome && name[0] == '.')
    return "it is a settings folder";

  // the head folder, and the language folders inside it
  if (isInside(root, source))
    return "it holds the head folder";
  if (fs::exists(root) && fs::exists(parent) && fs::equivalent(parent, root))
    return "it is a language folder of MAPS";
  return "";
}

// moves a project folder into the head folder: head folder / language / name
void importProject(const string &path, const string &lang) {
  if (!isValidName(lang)) {
    cout << "Error: invalid language.\n";
    return;
  }
  fs::path source = fs::absolute(path).lexically_normal();
  if (!source.has_filename()) // remove a trailing slash
    source = source.parent_path();
  if (!fs::is_directory(source)) {
    cout << "Error: that folder does not exist.\n";
    return;
  }
  fs::path root = loadRoot();
  string reason = protectedReason(source, root);
  if (reason != "") {
    cout << "Error: this folder is protected, " << reason << ".\n";
    return;
  }
  fs::path dest = root / languageFolder(lang) / source.filename();
  if (fs::exists(dest) && fs::equivalent(dest, source)) {
    writeFile(dest / MARKER, "created by MAPS\n"); // already in place
    cout << "Added " << dest.string() << "\n";
    return;
  }
  if (fs::exists(dest)) {
    cout << "Error: a project with this name already exists in "
         << dest.parent_path().string() << "\n";
    return;
  }
  cout << "Move from: " << source.string() << "\n";
  cout << "Move to  : " << dest.string() << "\n";
  if (!confirm("Move the folder?")) {
    cout << "Cancelled.\n";
    return;
  }
  fs::create_directories(dest.parent_path());
  error_code ec;
  fs::rename(source, dest, ec);
  if (ec) {
    cout << "Error: could not move the folder. Close any program that uses it. "
            "The new place must be on the same drive.\n";
    return;
  }
  writeFile(dest / MARKER, "created by MAPS\n");
  cout << "Imported " << dest.string() << "\n";
}

// prints each folder and the folders inside it (used for templates)
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

// shows only folders that hold the marker file
void listProjects() {
  fs::path root = loadRoot();
  if (!fs::exists(root)) {
    cout << "No projects yet.\n";
    return;
  }
  auto skip = fs::directory_options::skip_permission_denied;
  bool found = false;
  for (const auto &langDir : fs::directory_iterator(root, skip)) {
    if (!langDir.is_directory())
      continue;
    bool printedLang = false;
    for (const auto &projectDir :
         fs::directory_iterator(langDir.path(), skip)) {
      if (!projectDir.is_directory() || !fs::exists(projectDir.path() / MARKER))
        continue;
      if (!printedLang) {
        cout << langDir.path().filename().string() << "\n";
        printedLang = true;
      }
      cout << "\t" << projectDir.path().filename().string() << "\n";
      found = true;
    }
  }
  if (!found)
    cout << "No projects yet.\n";
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

// changes the head folder, only for a full path of a folder that already exists
void setRoot(const string &path) {
  fs::path root = fs::path(path).lexically_normal();
  if (!root.has_filename()) // remove a trailing slash
    root = root.parent_path();
  if (!root.is_absolute()) {
    cout << "Error: use the full path, for example C:\\Users\\name\\Projects\n";
    return;
  }
  if (!fs::is_directory(root)) {
    cout << "Error: that folder does not exist. Make it first, then try "
            "again.\n";
    return;
  }
  if (root == root.root_path() || root == exeDir()) {
    cout << "Error: do not use a drive root or the program folder.\n";
    return;
  }
  cout << "Old head folder: " << loadRoot().string() << "\n";
  cout << "New head folder: " << root.string() << "\n";
  cout << "Old projects stay where they are.\n";
  if (!confirm("Change it?")) {
    cout << "Cancelled.\n";
    return;
  }
  ofstream out(configFile());
  out << root.string() << "\n";
  cout << "Head folder set.\n";
}

// removes the saved setting, the default folder is used again
void resetRoot() {
  fs::remove(configFile());
  cout << "Head folder is the default again: " << loadRoot().string() << "\n";
}

int main(int argc, char *argv[]) {
  // 1. Check if the user passed any arguments at all (prevents crash)
  if (argc < 2) {
    cout << "ERROR: maps expected an argument. Try -> maps -h\n";
    return 0;
  }

  string arg1 = argv[1];

  // 2. Check for help flags
  if (arg1 == "-h" || arg1 == "--help" || arg1 == "?") {
    printHelp();
    return 0;
  }

  // 3. Process normal commands
  string command = arg1;
  try {
    if (command == "new" && (argc == 4 || argc == 5))
      newProject(argv[2], argv[3], argc == 5 ? argv[4] : "default");
    else if (command == "import" && argc == 4)
      importProject(argv[2], argv[3]);
    else if (command == "list")
      listProjects();
    else if (command == "templates")
      listTemplates();
    else if (command == "root" && argc == 2)
      cout << loadRoot().string() << "\n";
    else if (command == "root" && argc == 3 && string(argv[2]) == "reset")
      resetRoot();
    else if (command == "root" && argc == 3)
      setRoot(argv[2]);
    else if (command == "help")
      printHelp();
    else if (command == "info" && argc == 2) {
      showInfo();
    } else {
      cout << "\nUnknown command or wrong arguments.\n";
      printHelp();
      return 1;
    }
  } catch (const exception &e) {
    cout << "Error: " << e.what() << "\n";
    return 1;
  }
  return 0;
}