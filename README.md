<div align="center">

# MAPS Lite

**Modular Architecture Project Scaffolder**

A menu-based tool that makes and organizes project folders by language.

Lite is the demo version for a school project.

</div>

---

## Introduction

For a beginner, it is a common struggle to move between project folders and to set up a new project. This causes confusion and disorganization. Class projects and personal projects both have this problem.

MAPS solves this with a fixed structure and a small set of commands. It keeps every project in one place, sorted by language. It makes the first files of a new project from a template.

```
head folder \ language folder \ project folder
MAPS\C++\calc
```

MAPS helps new programmers to be responsible with the location and the architecture of their projects.

The design follows command-line tools such as npm. MAPS uses fewer and simpler commands.

## Commands

The **head folder** holds all projects. The default is `%USERPROFILE%\MAPS`.

LITE opens a numbered menu when you start the program. You choose an option by its number.

| Option | Action |
|---|---|
| 1 | New project. MAPS asks for the name and the language. |
| 2 | Import a project. MAPS asks for the folder path and the language. |
| 3 | List the projects. On Windows, you can open a project in File Explorer by its number. |
| 4 | Show the available languages and templates. |
| 5 | Show the head folder. |
| 6 | Change the head folder. MAPS asks for the full path. |
| 7 | Reset the head folder to the default. |
| 8 | Show the info screen. |
| 9 | Show the help text. |
| 0 | Exit. |

Maps Lite has four built-in languages. The names are not case-sensitive.

| Language | Names you can enter | Files |
|---|---|---|
| HTML | `html`, `htm` | `index.html`, `style.css` |
| C | `c` | `src/main.c`, `Makefile` |
| C++ | `cpp`, `c++`, `cxx`, `cc` | `src/main.cpp`, `Makefile` |
| PY | `py`, `python` | `main.py` |

Each project also gets a `README.md` and the marker file `.maps`. The text `{{name}}` in a built-in template becomes the project name.

### Safety rules

- **Head folder:** the path must be a full path to a folder that exists. MAPS shows the old and the new folder and asks for `y`.
- **Import:** MAPS shows the old and the new path and asks for `y`. It refuses system folders, program folders, the home folder, and user folders such as `Documents` and `Desktop`.
- **Projects:** MAPS does not replace an existing project.

## Limitations

- MAPS runs on Windows only.
- MAPS does not detect the language of a project. The user gives the language.
- MAPS does not delete projects.
- The list shows only projects that have the marker file `.maps`. It looks two levels down: head folder, language folder, project folder.
- The marker file `.maps` is not hidden on Windows.
- Import moves the folder. It does not copy it. The old place and the new place must be on the same drive. A program that uses the folder, for example an editor, can stop the move.
- Maps Lite has only four languages: HTML, C, C++ and PY. MAPS refuses any other language.
- Maps Lite has no template types, no `aliases.txt` and no template folders. A new language needs a code change and a new build.
- Maps Lite has no command mode. The program always opens the menu.
- Open in File Explorer works on Windows only.

## AI usage

This project uses AI-generated code. Most of the source code and the documentation drafts were written with AI tools, mainly Claude (Anthropic).

The team reviewed and edited all AI-generated code. The team also built the program and tested it on Windows.

The team is responsible for the final project.

## License

GNU General Public License v3.0 | https://choosealicense.com/licenses/gpl-3.0/.

## Credits

- Jasper Kyle F. Balinas
- Rafael Dominic C. Gabriel
- Terence Jerrome Montecillo
- Rionne J. Punto

M.A.P.S by Taurus Software.
FEU Institute of Technology.
Creation date: 11/12/2026.