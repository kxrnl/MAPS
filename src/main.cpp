/*

Main MAPS - Full tool version
Yes there is a lite version of this tool go find it

M.A.P.S by Taurus Software
Creation date: 11/12/2026
FEU Institute of Technology

*/

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
// #include <sstream>
#include <string>
#include <vector>

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

struct TemplateFile {
  string relativePath;
  string content;
};

struct LanguageTemplate {
  string folderName;
  vector<string> aliases;
  string description;
  vector<TemplateFile> files;
};

fs::path exeDir() {
  wchar_t buffer[MAX_PATH];
  DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  if (length > 0 && length < MAX_PATH)
    return fs::path(buffer).parent_path();
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

void writeFile(const fs::path &path, const string &content) {
  if (path.has_parent_path())
    fs::create_directories(path.parent_path());
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

// hardcoded templates for HTML, C, C++, PY, and WEB
const vector<LanguageTemplate> &getTemplates() {
  static const vector<LanguageTemplate> templates = {
      {
          "HTML",
          {"html", "htm"},
          "index.html, style.css",
          {
              {"index.html",
               "<!DOCTYPE html>\n"
               "<html lang=\"en\">\n"
               "<head>\n"
               "  <meta charset=\"UTF-8\" />\n"
               "  <meta name=\"viewport\" content=\"width=device-width, "
               "initial-scale=1.0\" />\n"
               "  <title>{{name}}</title>\n"
               "  <link rel=\"stylesheet\" href=\"style.css\" />\n"
               "</head>\n"
               "<body>\n"
               "  <h1>{{name}}</h1>\n"
               "</body>\n"
               "</html>\n"},
              {"style.css", "body {\n"
                            "  font-family: system-ui, sans-serif;\n"
                            "  margin: 2rem;\n"
                            "}\n"},
          },
      },
      {
          "C",
          {"c"},
          "src/main.c, Makefile",
          {
              {"src/main.c", "#include <stdio.h>\n\n"
                             "int main(void) {\n"
                             "  printf(\"Hello from {{name}}!\\n\");\n"
                             "  return 0;\n"
                             "}\n"},
              {"Makefile", "CC = gcc\n"
                           "CFLAGS = -Wall -Wextra -std=c17\n"
                           "TARGET = {{name}}\n\n"
                           "all: $(TARGET)\n\n"
                           "$(TARGET): src/main.c\n"
                           "\t$(CC) $(CFLAGS) src/main.c -o $(TARGET)\n\n"
                           "clean:\n"
                           "\trm -f $(TARGET)\n"},
          },
      },
      {
          "C++",
          {"cpp", "c++", "cxx", "cc"},
          "src/main.cpp, Makefile",
          {
              {"src/main.cpp", "#include <iostream>\n\n"
                               "using namespace std;\n\n"
                               "int main() {\n"
                               "  cout << \"Hello from {{name}}!\\n\";\n"
                               "  return 0;\n"
                               "}\n"},
              {"Makefile", "CXX = g++\n"
                           "CXXFLAGS = -Wall -Wextra -std=c++17\n"
                           "TARGET = {{name}}\n\n"
                           "all: $(TARGET)\n\n"
                           "$(TARGET): src/main.cpp\n"
                           "\t$(CXX) $(CXXFLAGS) src/main.cpp -o $(TARGET)\n\n"
                           "clean:\n"
                           "\trm -f $(TARGET)\n"},
          },
      },
      {
          "PY",
          {"py", "python"},
          "main.py",
          {
              {"main.py", "def main() -> None:\n"
                          "    print(\"Hello from {{name}}!\")\n\n\n"
                          "if __name__ == \"__main__\":\n"
                          "    main()\n"},
          },
      },
      {
          "WEB",
          {"web", "js"},
          "index.html, css/style.css, js/app.js",
          {
              {"index.html",
               "<!DOCTYPE html>\n"
               "<html lang=\"en\">\n"
               "<head>\n"
               "  <meta charset=\"UTF-8\" />\n"
               "  <meta name=\"viewport\" content=\"width=device-width, "
               "initial-scale=1.0\" />\n"
               "  <title>{{name}}</title>\n"
               "  <link rel=\"stylesheet\" href=\"css/style.css\" />\n"
               "</head>\n"
               "<body>\n"
               "  <h1>{{name}}</h1>\n"
               "  <script src=\"js/app.js\"></script>\n"
               "</body>\n"
               "</html>\n"},
              {"css/style.css", "body {\n"
                                "  font-family: system-ui, sans-serif;\n"
                                "  margin: 2rem;\n"
                                "}\n"},
              {"js/app.js", "console.log(\"{{name}} initialized\");\n"},
          },
      },
  };
  return templates;
}

// returns pointer to matching LanguageTemplate, or nullptr if unknown
const LanguageTemplate *findTemplate(const string &input) {
  string key = toLower(trim(input));
  const auto &list = getTemplates();

  for (const auto &tpl : list) {
    if (toLower(tpl.folderName) == key)
      return &tpl;
    for (const auto &alias : tpl.aliases) {
      if (toLower(alias) == key)
        return &tpl;
    }
  }
  return nullptr;
}

void showInfo() {
  fs::path root = loadRoot();
  cout << "Head folder : " << root.string()
       << (fs::exists(root) ? "" : "  (not made yet)") << "\n";
  cout << "Templates   : built-in (HTML, C, C++, PY, WEB)\n";
  cout << "Settings    : " << configFile().string()
       << (fs::exists(configFile()) ? ""
                                    : "  (not set, default head folder in use)")
       << "\n";
  cout << "Marker file : " << MARKER << " (inside each project)\n";
}

void printHelp() {
  cout << "M.A.P.S - Modular Architecture Project Scaffolder" << endl;
  cout << "Current Head folder: " << loadRoot().string() << "\n" << endl;
  cout << "Available languages: HTML, C, C++, PY, WEB" << endl;
  cout << endl;
  cout << "Usage:" << endl;
  cout << "\tmaps new <project_name> <lang>   create a project (example: "
          "maps new calc cpp)"
       << endl;
  cout << "\tmaps import <path> <lang>        move a project folder into the "
          "head folder"
       << endl;
  cout << "\tmaps list                        show all projects" << endl;
  cout << "\tmaps templates                   show built-in language templates"
       << endl;
  cout << "\tmaps root                        show the head folder" << endl;
  cout << "\tmaps root <path>                 change the head folder (full "
          "path, folder must exist)"
       << endl;
  cout << "\tmaps root reset                  go back to the default head "
          "folder"
       << endl;
  cout << "\tmaps help                        show this help message" << endl;
  cout << "\tmaps info                        show important file/folder "
          "locations"
       << endl;
  cout << "\tmaps menumode                    enter interactive menu mode"
       << endl;
}

void newProject(const string &name, const string &lang) {
  if (!isValidName(name) || !isValidName(lang)) {
    cout << "Error: invalid name.\n";
    return;
  }

  const LanguageTemplate *tpl = findTemplate(lang);
  if (tpl == nullptr) {
    cout << "Error: unknown language '" << lang
         << "'. Available: HTML, C, C++, PY, WEB\n";
    return;
  }

  fs::path project = loadRoot() / tpl->folderName / name;
  if (fs::exists(project)) {
    cout << "Error: project already exists.\n";
    return;
  }

  fs::create_directories(project);
  for (const auto &file : tpl->files) {
    fs::path target = project / render(file.relativePath, name);
    writeFile(target, render(file.content, name));
  }

  if (!fs::exists(project / "README.md"))
    writeFile(project / "README.md", "# " + name + "\n");
  writeFile(project / MARKER, "created by MAPS\n");
  cout << "Created " << project.string() << " [" << tpl->folderName << "]\n";
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
  const LanguageTemplate *tpl = findTemplate(lang);
  if (tpl == nullptr) {
    cout << "Error: unknown language '" << lang
         << "'. Available: HTML, C, C++, PY, WEB\n";
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
  fs::path dest = root / tpl->folderName / source.filename();
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
  cout << "Available languages:\n";
  for (const auto &tpl : getTemplates()) {
    cout << "  " << tpl.folderName << " -> " << tpl.description << "\n";
  }
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

// prompts for a single line of text (keeps spaces intact)
string promptLine(const string &label) {
  cout << label;
  string value;
  getline(cin, value);
  return trim(value);
}

void printMenu() {
  cout << "\n--- M.A.P.S Menu ---\n";
  cout << "[1] New Project\n";
  cout << "[2] Import Project\n";
  cout << "[3] List Projects\n";
  cout << "[4] Available Languages & Templates\n";
  cout << "[5] Show Head Folder\n";
  cout << "[6] Change Head Folder\n";
  cout << "[7] Reset Head Folder\n";
  cout << "[8] Info\n";
  cout << "[9] Help\n";
  cout << "[0] Exit\n";
}

// interactive numbered menu started by "maps menumode"
void runMenuMode() {
  bool cont = true;
  string choice;

  cout << "M.A.P.S Interactive Menu Mode\n";

  do {
    printMenu();
    cout << "Select an option: ";
    if (!getline(cin, choice)) {
      break;
    }

    choice = trim(choice);
    if (choice.empty()) {
      continue;
    }

    try {
      if (choice == "1") {
        string name = promptLine("Project name: ");
        string lang = promptLine("Language (HTML, C, C++, PY, WEB): ");
        newProject(name, lang);
      } else if (choice == "2") {
        string path = promptLine("Folder path to import: ");
        string lang = promptLine("Language (HTML, C, C++, PY, WEB): ");
        importProject(path, lang);
      } else if (choice == "3") {
        listProjects();
      } else if (choice == "4") {
        listTemplates();
      } else if (choice == "5") {
        cout << loadRoot().string() << "\n";
      } else if (choice == "6") {
        string path = promptLine("New head folder (full path): ");
        setRoot(path);
      } else if (choice == "7") {
        resetRoot();
      } else if (choice == "8") {
        showInfo();
      } else if (choice == "9") {
        printHelp();
      } else if (choice == "0" || toLower(choice) == "exit" ||
                 toLower(choice) == "quit") {
        cont = false;
      } else {
        cout << "Invalid option. Enter a number from 0 to 9.\n";
      }
    } catch (const exception &e) {
      cout << "Error: " << e.what() << "\n";
    }
  } while (cont != false);

  cout << "Exited menu mode.\n";
}

// runs a single command from CLI arguments
int runCliCommand(int argc, char *argv[]) {
  string arg1 = argv[1];

  if (arg1 == "-h" || arg1 == "--help" || arg1 == "?") {
    printHelp();
    return 0;
  }

  try {
    if (arg1 == "new" && argc == 4)
      newProject(argv[2], argv[3]);
    else if (arg1 == "import" && argc == 4)
      importProject(argv[2], argv[3]);
    else if (arg1 == "list")
      listProjects();
    else if (arg1 == "templates")
      listTemplates();
    else if (arg1 == "root" && argc == 2)
      cout << loadRoot().string() << "\n";
    else if (arg1 == "root" && argc == 3 && string(argv[2]) == "reset")
      resetRoot();
    else if (arg1 == "root" && argc == 3)
      setRoot(argv[2]);
    else if (arg1 == "help")
      printHelp();
    else if (arg1 == "info" && argc == 2) {
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

int main(int argc, char *argv[]) {
  // 1. Check if the user passed any arguments at all (prevents crash)
  if (argc < 2) {
    cout << "ERROR: maps expected an argument. Try -> maps -h or maps "
            "menumode\n";
    return 0;
  }

  string arg1 = argv[1];

  // 2. Enter interactive menu mode
  if (toLower(arg1) == "menumode") {
    runMenuMode();
    return 0;
  }

  // 3. Process normal CLI commands
  return runCliCommand(argc, argv);
}