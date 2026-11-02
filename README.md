# Academic Prerequisite and Course Registration Validator

A beginner-friendly C++17 console mini-project for a Data Structures course.

## What it does

- Stores courses in an array-based hash table using **linear probing**.
- Stores each course's prerequisite IDs in a **linked list**, so a course can have multiple prerequisites.
- Stores students in a **singly linked list** and each student's completed courses in another linked list.
- Checks all direct and indirect prerequisites before reporting a registration as accepted or rejected.
- Prevents a new prerequisite relationship from creating a circular dependency, using a **queue, a visited array, and loops**. It does not use DFS or recursion.
- Saves course, prerequisite, student, and completed-course records in sequential text files.
- Appends each registration decision to a registration-history file.

## Files in this folder

| File | Purpose |
|---|---|
| `main.cpp` | Complete source code, with comments |
| `courses.txt` | Course IDs and names |
| `prerequisites.txt` | One prerequisite relationship per line: `courseID|prerequisiteID` |
| `students.txt` | Student IDs and names |
| `completed.txt` | Completed-course records: `studentID|courseID` |
| `registrations.txt` | Append-only registration decision history |
| `TEAM_GUIDE.md` | Beginner explanation, algorithms, complexity, presentation plan, and viva questions |

The text files contain sample data so the program is ready for demonstration. The program loads them when it starts. It saves the main data files after changes, so make a backup before experimenting with important records.

## How to compile and run

### Windows (g++ / MinGW)

Open a terminal **in this project folder** and run:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o validator.exe
.\validator.exe
```

### Linux / macOS

```bash
g++ -std=c++17 -Wall -Wextra -pedantic main.cpp -o validator
./validator
```

Keep the terminal's current directory set to this folder when running the program. The data files are opened using relative filenames.

## Quick demonstration

The starter data includes these relationships:

- C201 requires C101.
- C202 requires C101.
- C301 requires both C201 and C202.
- C401 requires C301.
- C402 requires C201.

Student examples:

- `S001` has completed C101, C201, and C202. Checking S001 for C301 should be **accepted**.
- `S002` has completed only C101. Checking S002 for C301 should be **rejected**, listing C201 and C202 as missing.
- Try adding `C101` as a course that requires `C301`. The program should reject the relationship because it would create a cycle.

## Notes / scope

This is an educational mini-project, not a production college registration platform. It has a fixed hash-table capacity of 101 courses, does not support deleting courses or students, and uses `|` as a separator in text files. IDs and names entered by users should not contain `|`. An accepted decision is recorded in the history file; the program is a prerequisite validator and demonstration, not a complete enrollment-management system.
