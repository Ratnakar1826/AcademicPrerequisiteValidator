#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <limits>
#include <cctype>

using namespace std;

// ================================================================
// ACADEMIC PREREQUISITE AND COURSE REGISTRATION VALIDATOR
//
// Data structures used:
// 1. Array-based hash table with linear probing: stores courses.
// 2. Singly linked lists: store prerequisites and student records.
// 3. Queue + visited array + loops: checks whether adding a
//    prerequisite would create a cycle. No DFS or recursion is used.
// 4. Sequential text files: save and reload records.
// ================================================================

const int TABLE_SIZE = 101; // Fixed size for this mini-project.

// One node in a course's prerequisite linked list.
struct PrerequisiteNode {
    string courseID;
    PrerequisiteNode* next;
};

// One occupied entry in the course hash table.
struct Course {
    bool occupied = false;
    string courseID;
    string courseName;
    PrerequisiteNode* prerequisiteHead = nullptr;
};

// One node in a student's completed-course linked list.
struct CompletedCourseNode {
    string courseID;
    CompletedCourseNode* next;
};

// One student in the student linked list.
struct Student {
    string studentID;
    string studentName;
    CompletedCourseNode* completedHead = nullptr;
    Student* next = nullptr;
};

// Main in-memory storage.
Course courseTable[TABLE_SIZE];
Student* studentHead = nullptr;

// Forward declaration: record-changing functions save data automatically.
bool saveAllData();

// ------------------------- Utility functions ----------------------

// Remove spaces/tabs from the beginning and end of input.
string trim(const string& text) {
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return "";
    }
    size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// IDs are made uppercase so that c101 and C101 are treated as equal.
string normalizeID(string id) {
    id = trim(id);
    for (char& ch : id) {
        ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));
    }
    return id;
}

// Our simple text file format uses | between fields, so IDs/names
// should not contain this character.
bool isValidField(const string& value) {
    return !trim(value).empty() && value.find('|') == string::npos;
}

// Split one line such as C201|C101 into separate fields.
vector<string> splitPipe(const string& line) {
    vector<string> fields;
    string field;
    stringstream stream(line);
    while (getline(stream, field, '|')) {
        fields.push_back(trim(field));
    }
    return fields;
}

string readLine(const string& prompt) {
    cout << prompt;
    string value;
    getline(cin, value);
    return trim(value);
}

// -------------------------- Hash table ----------------------------

// Hash function: add the ASCII values of all ID characters and take
// the remainder after dividing by TABLE_SIZE.
int hashFunction(const string& rawID) {
    string id = normalizeID(rawID);
    unsigned long long sum = 0;
    for (unsigned char ch : id) {
        sum += ch;
    }
    return static_cast<int>(sum % TABLE_SIZE);
}

// Search for a course. Return its table index, or -1 if not found.
// Because this program never deletes courses, stopping at the first
// empty slot is safe when linear probing is used.
int findCourseIndex(const string& rawID) {
    string id = normalizeID(rawID);
    if (id.empty()) {
        return -1;
    }

    int start = hashFunction(id);
    for (int step = 0; step < TABLE_SIZE; ++step) {
        int index = (start + step) % TABLE_SIZE;

        if (!courseTable[index].occupied) {
            return -1;
        }
        if (courseTable[index].courseID == id) {
            return index;
        }
    }
    return -1;
}

Course* findCourse(const string& id) {
    int index = findCourseIndex(id);
    if (index == -1) {
        return nullptr;
    }
    return &courseTable[index];
}

// Insert one course into the hash table using LINEAR PROBING.
// This function returns false for invalid/duplicate IDs or a full table.
bool addCourseRecord(const string& rawID, const string& rawName, bool saveAfter) {
    string id = normalizeID(rawID);
    string name = trim(rawName);

    if (!isValidField(id) || !isValidField(name) || id.find(' ') != string::npos) {
        return false;
    }
    if (findCourseIndex(id) != -1) {
        return false; // Course IDs must be unique.
    }

    int start = hashFunction(id);
    for (int step = 0; step < TABLE_SIZE; ++step) {
        int index = (start + step) % TABLE_SIZE;
        if (!courseTable[index].occupied) {
            courseTable[index].occupied = true;
            courseTable[index].courseID = id;
            courseTable[index].courseName = name;
            courseTable[index].prerequisiteHead = nullptr;

            if (saveAfter) {
                if (!saveAllData()) {
                    cout << "Warning: course added in memory, but saving to files failed.\n";
                }
            }
            return true;
        }
    }
    return false; // No empty slot was available.
}

// ----------------------- Student linked list ----------------------

Student* findStudent(const string& rawID) {
    string id = normalizeID(rawID);
    Student* current = studentHead;
    while (current != nullptr) {
        if (current->studentID == id) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

bool addStudentRecord(const string& rawID, const string& rawName, bool saveAfter) {
    string id = normalizeID(rawID);
    string name = trim(rawName);

    if (!isValidField(id) || !isValidField(name) || id.find(' ') != string::npos) {
        return false;
    }
    if (findStudent(id) != nullptr) {
        return false;
    }

    // Insert at the head of the student linked list.
    Student* fresh = new Student;
    fresh->studentID = id;
    fresh->studentName = name;
    fresh->completedHead = nullptr;
    fresh->next = studentHead;
    studentHead = fresh;

    if (saveAfter) {
        if (!saveAllData()) {
            cout << "Warning: student added in memory, but saving to files failed.\n";
        }
    }
    return true;
}

bool hasCompletedCourse(const Student* student, const string& rawCourseID) {
    if (student == nullptr) {
        return false;
    }
    string id = normalizeID(rawCourseID);
    const CompletedCourseNode* current = student->completedHead;
    while (current != nullptr) {
        if (current->courseID == id) {
            return true;
        }
        current = current->next;
    }
    return false;
}

bool markCourseCompletedRecord(const string& rawStudentID,
                               const string& rawCourseID,
                               bool saveAfter) {
    string studentID = normalizeID(rawStudentID);
    string courseID = normalizeID(rawCourseID);
    Student* student = findStudent(studentID);

    if (student == nullptr || findCourse(courseID) == nullptr) {
        return false;
    }
    if (hasCompletedCourse(student, courseID)) {
        return false; // Do not add the same completion twice.
    }

    CompletedCourseNode* fresh = new CompletedCourseNode;
    fresh->courseID = courseID;
    fresh->next = student->completedHead;
    student->completedHead = fresh;

    if (saveAfter) {
        if (!saveAllData()) {
            cout << "Warning: completion added in memory, but saving to files failed.\n";
        }
    }
    return true;
}

// ------------------- Prerequisite linked lists --------------------

bool alreadyHasPrerequisite(const Course* course, const string& rawPrereqID) {
    if (course == nullptr) {
        return false;
    }
    string prereqID = normalizeID(rawPrereqID);
    const PrerequisiteNode* current = course->prerequisiteHead;
    while (current != nullptr) {
        if (current->courseID == prereqID) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// Return true if the prerequisite chain starting at startID can reach
// targetID. The search is performed with a queue and a visited array.
// This is an iterative traversal: it does NOT use DFS or recursion.
bool canReachCourse(const string& startID, const string& targetID) {
    string start = normalizeID(startID);
    string target = normalizeID(targetID);
    queue<string> workQueue;
    bool visited[TABLE_SIZE] = {false};

    workQueue.push(start);
    while (!workQueue.empty()) {
        string currentID = workQueue.front();
        workQueue.pop();

        // Check the target before expanding the current course.
        if (currentID == target) {
            return true;
        }

        int index = findCourseIndex(currentID);
        if (index == -1) {
            continue; // Ignore unknown data; validation reports it elsewhere.
        }
        if (visited[index]) {
            continue;
        }
        visited[index] = true;

        // Add every prerequisite of this course to the queue.
        PrerequisiteNode* prerequisite = courseTable[index].prerequisiteHead;
        while (prerequisite != nullptr) {
            workQueue.push(prerequisite->courseID);
            prerequisite = prerequisite->next;
        }
    }
    return false;
}

// Adding course A -> prerequisite B creates a cycle exactly when B's
// existing prerequisite relationships can already lead back to A.
bool addingPrerequisiteWouldCreateCycle(const string& courseID,
                                        const string& prerequisiteID) {
    if (normalizeID(courseID) == normalizeID(prerequisiteID)) {
        return true; // A course cannot be its own prerequisite.
    }
    return canReachCourse(prerequisiteID, courseID);
}

bool addPrerequisiteRecord(const string& rawCourseID,
                           const string& rawPrerequisiteID,
                           bool saveAfter) {
    string courseID = normalizeID(rawCourseID);
    string prerequisiteID = normalizeID(rawPrerequisiteID);
    Course* course = findCourse(courseID);

    if (course == nullptr || findCourse(prerequisiteID) == nullptr) {
        return false; // Both course IDs must already exist.
    }
    if (alreadyHasPrerequisite(course, prerequisiteID)) {
        return false; // Do not create duplicate links.
    }
    if (addingPrerequisiteWouldCreateCycle(courseID, prerequisiteID)) {
        return false; // Do not save an edge that causes a cycle.
    }

    // Insert the prerequisite node at the head of this course's list.
    PrerequisiteNode* fresh = new PrerequisiteNode;
    fresh->courseID = prerequisiteID;
    fresh->next = course->prerequisiteHead;
    course->prerequisiteHead = fresh;

    if (saveAfter) {
        if (!saveAllData()) {
            cout << "Warning: prerequisite added in memory, but saving to files failed.\n";
        }
    }
    return true;
}

// Check whether any currently stored prerequisite relationship is circular.
bool hasCircularPrerequisites() {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (!courseTable[i].occupied) {
            continue;
        }
        PrerequisiteNode* prerequisite = courseTable[i].prerequisiteHead;
        while (prerequisite != nullptr) {
            // If this prerequisite can eventually depend on the original
            // course, the existing relationship set contains a cycle.
            if (canReachCourse(prerequisite->courseID, courseTable[i].courseID)) {
                return true;
            }
            prerequisite = prerequisite->next;
        }
    }
    return false;
}

bool hasUnknownPrerequisiteReference() {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (!courseTable[i].occupied) {
            continue;
        }
        PrerequisiteNode* prerequisite = courseTable[i].prerequisiteHead;
        while (prerequisite != nullptr) {
            if (findCourse(prerequisite->courseID) == nullptr) {
                return true;
            }
            prerequisite = prerequisite->next;
        }
    }
    return false;
}

bool prerequisiteDataIsValid() {
    return !hasUnknownPrerequisiteReference() && !hasCircularPrerequisites();
}

// ------------------------- File handling --------------------------

// Save the in-memory structures to sequential text files. Records are
// separated by | to keep the file format easy to read for beginners.
// These files must be in the program's current working directory.
bool saveAllData() {
    ofstream coursesFile("courses.txt");
    ofstream prerequisitesFile("prerequisites.txt");
    ofstream studentsFile("students.txt");
    ofstream completedFile("completed.txt");

    if (!coursesFile || !prerequisitesFile || !studentsFile || !completedFile) {
        return false;
    }

    // Save the course hash table and each course's prerequisite list.
    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (!courseTable[i].occupied) {
            continue;
        }
        coursesFile << courseTable[i].courseID << '|'
                    << courseTable[i].courseName << '\n';

        PrerequisiteNode* prerequisite = courseTable[i].prerequisiteHead;
        while (prerequisite != nullptr) {
            prerequisitesFile << courseTable[i].courseID << '|'
                              << prerequisite->courseID << '\n';
            prerequisite = prerequisite->next;
        }
    }

    // Save students and their completed-course linked lists.
    Student* student = studentHead;
    while (student != nullptr) {
        studentsFile << student->studentID << '|'
                     << student->studentName << '\n';

        CompletedCourseNode* completed = student->completedHead;
        while (completed != nullptr) {
            completedFile << student->studentID << '|'
                          << completed->courseID << '\n';
            completed = completed->next;
        }
        student = student->next;
    }

    coursesFile.close();
    prerequisitesFile.close();
    studentsFile.close();
    completedFile.close();

    return !coursesFile.fail() && !prerequisitesFile.fail() &&
           !studentsFile.fail() && !completedFile.fail();
}

// Load courses first, then students, then prerequisite relationships,
// and finally completed courses. The order matters because later records
// refer to course and student IDs loaded earlier.
void loadAllData() {
    string line;

    ifstream coursesFile("courses.txt");
    if (coursesFile) {
        while (getline(coursesFile, line)) {
            if (trim(line).empty()) continue;
            vector<string> fields = splitPipe(line);
            if (fields.size() == 2 && addCourseRecord(fields[0], fields[1], false)) {
                // Successfully loaded.
            } else {
                cout << "Warning: skipped invalid/duplicate course row: " << line << '\n';
            }
        }
    }

    ifstream studentsFile("students.txt");
    if (studentsFile) {
        while (getline(studentsFile, line)) {
            if (trim(line).empty()) continue;
            vector<string> fields = splitPipe(line);
            if (fields.size() == 2 && addStudentRecord(fields[0], fields[1], false)) {
                // Successfully loaded.
            } else {
                cout << "Warning: skipped invalid/duplicate student row: " << line << '\n';
            }
        }
    }

    ifstream prerequisitesFile("prerequisites.txt");
    if (prerequisitesFile) {
        while (getline(prerequisitesFile, line)) {
            if (trim(line).empty()) continue;
            vector<string> fields = splitPipe(line);
            if (fields.size() != 2 ||
                !addPrerequisiteRecord(fields[0], fields[1], false)) {
                cout << "Warning: skipped invalid, duplicate, unknown, or circular prerequisite row: "
                     << line << '\n';
            }
        }
    }

    ifstream completedFile("completed.txt");
    if (completedFile) {
        while (getline(completedFile, line)) {
            if (trim(line).empty()) continue;
            vector<string> fields = splitPipe(line);
            if (fields.size() != 2 ||
                !markCourseCompletedRecord(fields[0], fields[1], false)) {
                cout << "Warning: skipped invalid/duplicate completion row: " << line << '\n';
            }
        }
    }
}

// Registration decisions are appended so earlier decisions are not lost.
void appendRegistrationLog(const string& studentID,
                           const string& courseID,
                           const string& status,
                           const string& reason) {
    ofstream logFile("registrations.txt", ios::app);
    if (!logFile) {
        cout << "Warning: unable to write registrations.txt.\n";
        return;
    }
    // The log is also sequential: one decision per line.
    logFile << studentID << '|' << courseID << '|'
            << status << '|' << reason << '\n';
}

// ---------------------- Registration validation -------------------

// Collect every direct and indirect prerequisite not completed by the
// student. A queue explores prerequisite relationships level by level.
// The visited array prevents a prerequisite shared by multiple paths from
// being checked/reported more than once.
vector<string> collectMissingPrerequisites(Student* student,
                                           const string& requestedCourseID) {
    vector<string> missing;
    queue<string> workQueue;
    bool visited[TABLE_SIZE] = {false};

    Course* requestedCourse = findCourse(requestedCourseID);
    if (requestedCourse == nullptr) {
        return missing;
    }

    // Build a fast lookup array for this student's completed courses.
    // Each course hash-table slot is either completed or not completed.
    bool completedFlags[TABLE_SIZE] = {false};
    CompletedCourseNode* completedNode = student->completedHead;
    while (completedNode != nullptr) {
        int completedIndex = findCourseIndex(completedNode->courseID);
        if (completedIndex != -1) {
            completedFlags[completedIndex] = true;
        }
        completedNode = completedNode->next;
    }

    PrerequisiteNode* first = requestedCourse->prerequisiteHead;
    while (first != nullptr) {
        workQueue.push(first->courseID);
        first = first->next;
    }

    while (!workQueue.empty()) {
        string currentID = workQueue.front();
        workQueue.pop();

        int index = findCourseIndex(currentID);
        if (index == -1 || visited[index]) {
            continue;
        }
        visited[index] = true;

        // A prerequisite must be completed, not merely registered.
        // The completedFlags array makes this check constant-time.
        if (!completedFlags[index]) {
            missing.push_back(currentID);
        }

        // Also examine prerequisites of this prerequisite.
        PrerequisiteNode* nextPrerequisite = courseTable[index].prerequisiteHead;
        while (nextPrerequisite != nullptr) {
            workQueue.push(nextPrerequisite->courseID);
            nextPrerequisite = nextPrerequisite->next;
        }
    }
    return missing;
}

string buildMissingReason(const vector<string>& missing) {
    string reason = "Missing prerequisites: ";
    for (size_t i = 0; i < missing.size(); ++i) {
        Course* course = findCourse(missing[i]);
        if (i > 0) reason += ", ";
        reason += missing[i];
        if (course != nullptr) {
            reason += " (" + course->courseName + ")";
        }
    }
    return reason;
}

void checkRegistration() {
    string studentID = normalizeID(readLine("Enter student ID: "));
    string courseID = normalizeID(readLine("Enter course ID to register for: "));

    Student* student = findStudent(studentID);
    Course* course = findCourse(courseID);

    if (student == nullptr) {
        cout << "REGISTRATION REJECTED: student ID was not found.\n";
        appendRegistrationLog(studentID, courseID, "REJECTED", "Student ID not found");
        return;
    }
    if (course == nullptr) {
        cout << "REGISTRATION REJECTED: course ID was not found.\n";
        appendRegistrationLog(studentID, courseID, "REJECTED", "Course ID not found");
        return;
    }
    if (!prerequisiteDataIsValid()) {
        cout << "REGISTRATION REJECTED: prerequisite data is invalid or circular.\n";
        appendRegistrationLog(studentID, courseID, "REJECTED",
                              "Prerequisite definitions are invalid or circular");
        return;
    }
    if (hasCompletedCourse(student, courseID)) {
        cout << "REGISTRATION REJECTED: this student has already completed the course.\n";
        appendRegistrationLog(studentID, courseID, "REJECTED",
                              "Student has already completed this course");
        return;
    }

    vector<string> missing = collectMissingPrerequisites(student, courseID);
    if (missing.empty()) {
        cout << "\nREGISTRATION ACCEPTED\n";
        cout << "Student: " << student->studentName << " (" << studentID << ")\n";
        cout << "Course:  " << course->courseName << " (" << courseID << ")\n";
        cout << "Reason:  All direct and indirect prerequisites are completed.\n";
        appendRegistrationLog(studentID, courseID, "ACCEPTED",
                              "All direct and indirect prerequisites are completed");
    } else {
        string reason = buildMissingReason(missing);
        cout << "\nREGISTRATION REJECTED\n";
        cout << "Student: " << student->studentName << " (" << studentID << ")\n";
        cout << "Course:  " << course->courseName << " (" << courseID << ")\n";
        cout << reason << '\n';
        cout << "Reason: required course(s) have not been completed.\n";
        appendRegistrationLog(studentID, courseID, "REJECTED", reason);
    }
}

// ------------------------- Display functions ----------------------

void displayAllCourses() {
    bool foundAny = false;
    cout << "\n========== COURSE CATALOGUE ==========\n";
    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (!courseTable[i].occupied) continue;
        foundAny = true;
        cout << "\n" << courseTable[i].courseID << " - "
             << courseTable[i].courseName << '\n';
        cout << "  Prerequisites: ";
        PrerequisiteNode* prerequisite = courseTable[i].prerequisiteHead;
        if (prerequisite == nullptr) {
            cout << "None";
        } else {
            while (prerequisite != nullptr) {
                Course* requiredCourse = findCourse(prerequisite->courseID);
                cout << prerequisite->courseID;
                if (requiredCourse != nullptr) {
                    cout << " (" << requiredCourse->courseName << ")";
                }
                if (prerequisite->next != nullptr) cout << ", ";
                prerequisite = prerequisite->next;
            }
        }
        cout << '\n';
    }
    if (!foundAny) {
        cout << "No courses are stored yet.\n";
    }
}

void displayStudentRecord() {
    string studentID = normalizeID(readLine("Enter student ID: "));
    Student* student = findStudent(studentID);
    if (student == nullptr) {
        cout << "Student not found.\n";
        return;
    }

    cout << "\nStudent: " << student->studentName << " (" << student->studentID << ")\n";
    cout << "Completed courses:\n";
    CompletedCourseNode* completed = student->completedHead;
    if (completed == nullptr) {
        cout << "  None recorded\n";
        return;
    }
    while (completed != nullptr) {
        Course* course = findCourse(completed->courseID);
        cout << "  - " << completed->courseID;
        if (course != nullptr) cout << " - " << course->courseName;
        cout << '\n';
        completed = completed->next;
    }
}

void validatePrerequisiteMenu() {
    if (hasUnknownPrerequisiteReference()) {
        cout << "INVALID: a prerequisite refers to a course that does not exist.\n";
    } else if (hasCircularPrerequisites()) {
        cout << "INVALID: circular prerequisite dependency detected.\n";
        cout << "The program normally prevents new circular relationships from being added.\n";
    } else {
        cout << "VALID: all prerequisite course IDs exist and no cycle was detected.\n";
    }
}

void viewRegistrationHistory() {
    ifstream logFile("registrations.txt");
    if (!logFile) {
        cout << "No registration history file exists yet.\n";
        return;
    }

    string line;
    bool foundAny = false;
    cout << "\n========== REGISTRATION HISTORY ==========\n";
    while (getline(logFile, line)) {
        if (trim(line).empty()) continue;
        vector<string> fields = splitPipe(line);
        foundAny = true;
        if (fields.size() >= 4) {
            cout << "Student: " << fields[0]
                 << " | Course: " << fields[1]
                 << " | Status: " << fields[2]
                 << " | Reason: " << fields[3] << '\n';
        } else {
            cout << line << '\n';
        }
    }
    if (!foundAny) cout << "No registration decisions have been recorded.\n";
}

// ----------------------- Memory cleanup ---------------------------

// Free every node allocated with new before the program exits.
void releaseMemory() {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (!courseTable[i].occupied) continue;
        PrerequisiteNode* prerequisite = courseTable[i].prerequisiteHead;
        while (prerequisite != nullptr) {
            PrerequisiteNode* next = prerequisite->next;
            delete prerequisite;
            prerequisite = next;
        }
        courseTable[i].prerequisiteHead = nullptr;
    }

    while (studentHead != nullptr) {
        Student* nextStudent = studentHead->next;
        CompletedCourseNode* completed = studentHead->completedHead;
        while (completed != nullptr) {
            CompletedCourseNode* nextCompleted = completed->next;
            delete completed;
            completed = nextCompleted;
        }
        delete studentHead;
        studentHead = nextStudent;
    }
}

// --------------------------- Main menu ----------------------------

void showMenu() {
    cout << "\n====================================================\n";
    cout << " ACADEMIC PREREQUISITE & COURSE REGISTRATION VALIDATOR\n";
    cout << "====================================================\n";
    cout << "1.  Add course\n";
    cout << "2.  Add prerequisite to a course\n";
    cout << "3.  Display all courses and prerequisites\n";
    cout << "4.  Add student\n";
    cout << "5.  Mark a course as completed for a student\n";
    cout << "6.  Check registration eligibility\n";
    cout << "7.  Display a student's record\n";
    cout << "8.  Validate prerequisite relationships\n";
    cout << "9.  View registration history\n";
    cout << "10. Save and exit\n";
    cout << "Enter choice: ";
}

int main() {
    cout << "Loading saved course and student data...\n";
    loadAllData();
    cout << "Data loading complete.\n";

    bool running = true;
    while (running) {
        showMenu();
        int choice;
        if (!(cin >> choice)) {
            if (cin.eof()) break;
            cout << "Please enter a number from 1 to 10.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case 1: {
                string id = readLine("Enter new course ID (example C501): ");
                string name = readLine("Enter course name: ");
                if (addCourseRecord(id, name, true)) {
                    cout << "Course added successfully.\n";
                } else {
                    cout << "Could not add course. Check the ID/name, duplicate ID, and table capacity.\n";
                }
                break;
            }
            case 2: {
                string courseID = readLine("Enter the course ID that has a prerequisite: ");
                string prereqID = readLine("Enter prerequisite course ID: ");
                if (findCourse(courseID) == nullptr || findCourse(prereqID) == nullptr) {
                    cout << "Both course IDs must exist before linking them.\n";
                } else if (alreadyHasPrerequisite(findCourse(courseID), prereqID)) {
                    cout << "That prerequisite is already recorded.\n";
                } else if (addingPrerequisiteWouldCreateCycle(courseID, prereqID)) {
                    cout << "Rejected: this relationship would create a circular prerequisite.\n";
                } else if (addPrerequisiteRecord(courseID, prereqID, true)) {
                    cout << "Prerequisite added successfully.\n";
                } else {
                    cout << "Could not add prerequisite. Check the course IDs.\n";
                }
                break;
            }
            case 3:
                displayAllCourses();
                break;
            case 4: {
                string id = readLine("Enter student ID (example S004): ");
                string name = readLine("Enter student name: ");
                if (addStudentRecord(id, name, true)) {
                    cout << "Student added successfully.\n";
                } else {
                    cout << "Could not add student. Check the ID/name or duplicate ID.\n";
                }
                break;
            }
            case 5: {
                string studentID = readLine("Enter student ID: ");
                string courseID = readLine("Enter completed course ID: ");
                if (markCourseCompletedRecord(studentID, courseID, true)) {
                    cout << "Completed course recorded successfully.\n";
                } else {
                    cout << "Could not record completion. Check IDs or ensure it is not already recorded.\n";
                }
                break;
            }
            case 6:
                checkRegistration();
                break;
            case 7:
                displayStudentRecord();
                break;
            case 8:
                validatePrerequisiteMenu();
                break;
            case 9:
                viewRegistrationHistory();
                break;
            case 10:
                if (saveAllData()) {
                    cout << "All course/student data saved. Registration history is already appended separately.\n";
                } else {
                    cout << "Warning: one or more data files could not be saved.\n";
                }
                running = false;
                break;
            default:
                cout << "Invalid choice. Please enter a number from 1 to 10.\n";
        }
    }

    releaseMemory();
    cout << "Program closed.\n";
    return 0;
}
