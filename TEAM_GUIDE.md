# Team Guide: Academic Prerequisite and Course Registration Validator

This guide is written for team members who are learning the project from the basics. Read it alongside `main.cpp`.

## 1. The problem in simple English

Colleges offer courses. Some courses can only be taken after a student completes other courses. These earlier courses are called **prerequisites**.

Example:

- C101 = Programming Basics
- C201 = Object-Oriented Programming; requires C101
- C202 = Discrete Mathematics; requires C101
- C301 = Data Structures; requires both C201 and C202

A student who wants C301 must have completed C201 and C202. This project also checks the earlier prerequisites of those courses. Therefore, if C101 is missing from the student's completed record, it can be reported too.

The system must also prevent circular rules, for example:

`A requires B`, `B requires C`, and `C requires A`.

These rules are impossible to satisfy in a normal prerequisite sequence. The program prevents adding a relationship that would create this kind of cycle.

## 2. Important words

- **Course ID:** A short unique code, such as `C301`.
- **Prerequisite:** A course that must already be completed before another course can be taken.
- **Completed course:** A course recorded as already passed/completed by a student.
- **Hash table:** An array where a hash function helps decide where a record should be stored.
- **Collision:** Two IDs produce the same starting position in a hash table.
- **Linear probing:** When the starting slot is occupied, check the next slot, then the next, until a free slot is found.
- **Linked list:** Nodes connected by pointers. Each node refers to the next node.
- **Queue:** A First-In, First-Out (FIFO) structure. The item inserted first is removed first.
- **Cycle:** A path that eventually leads back to the same course.
- **Sequential file:** A file read or written record by record, from beginning to end.

## 3. Which data structure does what?

| Data structure | Where in the code | Why it is used |
|---|---|---|
| Array-based hash table | `courseTable[TABLE_SIZE]` | Finds course IDs without scanning every course in normal cases. |
| Linear probing | `hashFunction()`, `findCourseIndex()`, `addCourseRecord()` | Handles two course IDs that hash to the same position. |
| Prerequisite linked list | `PrerequisiteNode`, `Course::prerequisiteHead` | One course can have zero, one, or many prerequisites. |
| Student linked list | `Student`, `studentHead`, `findStudent()` | Students are stored as dynamically allocated nodes. |
| Completed-course linked list | `CompletedCourseNode`, `Student::completedHead` | Each student's completed courses can grow dynamically. |
| Queue | `queue<string>` in `canReachCourse()` and `collectMissingPrerequisites()` | Keeps track of prerequisite IDs still waiting to be checked. |
| Array | `visited[TABLE_SIZE]`, `completedFlags[TABLE_SIZE]` | Remembers which hash-table slots have been visited or completed. |
| Sequential text files | `saveAllData()`, `loadAllData()`, `appendRegistrationLog()` | Preserves data after the program closes. |

### About graphs and DFS

The prerequisite relationships can be described mathematically as a directed graph, because one course can point to its prerequisite courses. However, this implementation does not create a separate `Graph` class or use DFS. Each course stores its prerequisites in a linked list, and the program follows those lists with a queue, a visited array, and loops. Be honest about this distinction if asked in viva.

## 4. How the hash table works

The project uses 101 slots. The hash function adds the character codes (ASCII values) of the uppercase course ID and calculates the remainder after division by 101:

`hash position = sum of character codes % 101`

For example, `C101` gives a starting slot based on its character codes. If that slot is already occupied by a different course, linear probing checks the next slot. The search follows the same sequence, so it can locate a course after a collision.

Why use a hash table? A course ID is unique and is often searched during registration. Hashing is suited to searching by a key such as a course ID.

**Complexity:** expected/average lookup and insertion are O(1) when the table is not crowded and the hash distribution is reasonable. The worst case is O(N), where N is the number of stored courses. Here the capacity is fixed at 101 slots.

## 5. How linked lists work in this project

A linked-list node stores data and a pointer to the next node. The final node points to `nullptr`.

### Prerequisite list

C301 has two prerequisite nodes:

`C301.prerequisiteHead -> C201 -> C202 -> nullptr`

The order in this display is not important. The two prerequisite IDs are stored as separate nodes. Adding another prerequisite adds another node rather than needing a new fixed-size field.

### Student list

`studentHead` points to the first student. Every student has a `next` pointer to the next student. Each student also has `completedHead`, which points to that student's completed-course list.

### Why pointers are needed

The number of prerequisites, students, and completed courses is not fixed in advance. Pointers allow the program to connect dynamically allocated nodes. The program also calls `releaseMemory()` at exit to free the nodes created with `new`.

## 6. How multiple prerequisites are checked

Suppose C301 requires C201 and C202, while both C201 and C202 require C101.

The program starts with C301's prerequisite list and places C201 and C202 in a queue. It removes a course ID from the front of the queue, checks whether the student completed it, and puts that course's own prerequisites into the queue. A visited array avoids examining the same course twice, which matters when different paths share the prerequisite C101.

Pseudocode:

```text
Make an empty queue
Make all visited flags false
Put every direct prerequisite of the requested course into the queue

While the queue is not empty:
    Remove the front course ID
    Find its hash-table slot
    If it was already visited, continue
    Mark it visited
    If the student has not completed it, add it to missing courses
    Put its prerequisites into the queue

If the missing list is empty, accept the request
Otherwise, reject the request and display the missing list
```

The code also builds `completedFlags[]` before checking. It maps completed course IDs to their hash-table slots so each completion check can be performed quickly.

## 7. How cycle detection works without DFS

When adding a new relationship `A requires B`, the program needs to check whether B already depends on A. If it does, the new relationship would complete a cycle.

The function `addingPrerequisiteWouldCreateCycle(A, B)` asks whether the existing prerequisite links starting at B can reach A. The helper `canReachCourse()` performs that search using a queue and `visited[]`.

Example existing relationships:

- C201 requires C101.
- C202 requires C101.
- C301 requires C201 and C202.

Now try to add `C101 requires C301`.

The search starts from C301, follows its prerequisites to C201/C202, and reaches C101. The proposed relationship is rejected because it would create a cycle.

The program checks for an existing route **before** allocating and attaching the new prerequisite node. That way the invalid relationship is never saved.

**Why a visited array?** A course can be reached through more than one path. Without a visited array, the program could repeat work unnecessarily, and a pre-existing cycle could cause an endless traversal. The array ensures each course slot is expanded at most once per search.

## 8. File storage

The project uses simple sequential text files that can be opened in Notepad:

| File | Format | Meaning |
|---|---|---|
| `courses.txt` | `C101|Programming Basics` | One course per line |
| `prerequisites.txt` | `C301|C201` | First ID is the course; second ID is one prerequisite |
| `students.txt` | `S001|Aarav Patil` | One student per line |
| `completed.txt` | `S001|C101` | Student S001 completed C101 |
| `registrations.txt` | `S001|C301|ACCEPTED|...` | One decision appended per line |

The program loads files in this order: courses, students, prerequisites, completed courses. This is important because prerequisite lines refer to course IDs and completion lines refer to both students and courses.

The first four files are rewritten from the in-memory data when a record changes. The registration history is append-only so prior decisions remain in the file.

## 9. Function map: where to find the code

| Function / structure | Role |
|---|---|
| `Course`, `PrerequisiteNode` | Course record and its prerequisite linked list |
| `hashFunction()` | Computes the initial hash-table slot |
| `findCourseIndex()` | Searches for a course using linear probing |
| `addCourseRecord()` | Inserts a course in the hash table |
| `Student`, `CompletedCourseNode` | Student record and completed-course list |
| `findStudent()` | Searches the student linked list |
| `addPrerequisiteRecord()` | Validates and adds a prerequisite node |
| `canReachCourse()` | Queue-based search through prerequisite lists |
| `addingPrerequisiteWouldCreateCycle()` | Determines whether the proposed link creates a cycle |
| `collectMissingPrerequisites()` | Finds missing direct and indirect requirements |
| `checkRegistration()` | Produces an accepted/rejected decision and writes the log |
| `saveAllData()` | Writes sequential data files |
| `loadAllData()` | Loads records from files in dependency order |
| `releaseMemory()` | Frees allocated linked-list nodes |
| `main()` | Loads data, displays menu, calls operations, saves and exits |

## 10. Complexity analysis

Let:
- N = number of courses
- E = number of prerequisite relationships
- S = number of students
- C = completed-course records for the student currently being checked

| Operation | Expected / worst-case complexity | Explanation |
|---|---|---|
| Course lookup | O(1) average; O(N) worst | Hash table with linear probing |
| Student lookup | O(S) | May traverse the student linked list |
| Add a student at list head | O(1) | No traversal is needed for insertion |
| Add a prerequisite after validation | O(N + E) expected for the reachability check | Queue can visit courses and prerequisite links; hash lookup is expected O(1) |
| Check all prerequisite requirements | O(C + N + E) expected | Build completion flags, then traverse reachable prerequisite links |
| Save/load records | O(number of records) | Sequentially visits each file record |

These are expected-time estimates for hash-table lookups; the hash table's worst case can be slower. Actual performance is not important for the small sample dataset, but being able to explain the reason for each estimate is important in viva.

## 11. Sample test cases for the demonstration

The included sample files contain six courses and three students.

| Test | What to do | Expected result |
|---|---|---|
| Successful check | Select 6; enter S001 and C301 | Accepted; S001 has C201 and C202, and both prerequisite chains lead to completed C101 |
| Missing prerequisites | Select 6; enter S002 and C301 | Rejected; C201 and C202 are listed as missing |
| Partial completion | Select 6; enter S003 and C301 | Rejected; C202 is missing |
| Cycle prevention | Select 2; enter course C101 and prerequisite C301 | Rejected because C301 already depends on C101 through its prerequisites |
| Multiple prerequisites | Select 3 | C301 displays both C201 and C202 |
| Unknown course | Try registering for C999 | Rejected with course-not-found reason |
| Duplicate course ID | Try adding C101 again | Insertion rejected |
| Persistence | Add a harmless new course, exit, restart, then display courses | New course remains because the file was saved |
| History | Make a registration decision, then select 9 | Decision appears in `registrations.txt` and on the screen |

For a clean demonstration, make a backup copy of the text files first. The program intentionally persists changes.

## 12. Suggested 13–14 minute presentation

1. **Problem statement (1 minute):** Explain why students should not register before completing prerequisites.
2. **Requirements (1 minute):** Courses, students, multiple prerequisites, missing-course reporting, cycle prevention, file storage.
3. **Data structures (2 minutes):** Hash table, linked lists, queue, array, sequential files.
4. **Hashing and linked lists (2 minutes):** Explain linear probing and show one course with two prerequisite nodes.
5. **Registration and cycle algorithms (2 minutes):** Explain the queue and visited array using a simple example.
6. **Live demo (3 minutes):** Show accepted S001/C301, rejected S002/C301, then attempt the circular relationship.
7. **Complexity and tests (1–2 minutes):** Give average/worst lookup and traversal complexity, then mention file persistence.
8. **Conclusion (30 seconds):** State that multiple data structures work together to solve one practical problem.

## 13. Viva questions and simple answers

**Q1. What is the purpose of this project?**  
It checks whether a student has completed all required prerequisite courses before accepting a course-registration request. It also detects circular prerequisite definitions and keeps a record of the decision.

**Q2. Why did you use a hash table for courses?**  
Courses have unique IDs. Hashing helps find a course directly by its ID without searching every course in the usual case.

**Q3. What is a collision?**  
A collision occurs when two different IDs produce the same initial hash-table position.

**Q4. How does linear probing resolve collisions?**  
It checks the next slot, then the next slot, until it finds the required course or an empty slot.

**Q5. Why use linked lists?**  
The number of students, prerequisites, and completed courses can vary. Linked lists allow records to be added dynamically.

**Q6. How does the program support multiple prerequisites?**  
Each course has a pointer to the first prerequisite node. Every node stores one prerequisite course ID and a pointer to the next node.

**Q7. What does the queue do?**  
It temporarily stores prerequisite IDs waiting to be checked. The program removes the oldest queued ID, checks it, and adds its prerequisites.

**Q8. What is the visited array for?**  
It records which course-table slots have already been expanded during a search. This avoids repeated work when prerequisite paths overlap.

**Q9. How do you detect a cycle without DFS?**  
Before adding A's prerequisite B, the program uses a queue to see whether B's existing prerequisite chain can reach A. If it can, adding A → B would close a cycle, so the program rejects it.

**Q10. Why is a cycle invalid?**        
It can create impossible or endlessly dependent requirements. For example, A requires B, B requires C, and C requires A.

**Q11. What if the student has completed a direct prerequisite but not an earlier prerequisite of that course?**  
The program checks the full chain, not just direct prerequisites, so the earlier missing course is also reported.

**Q12. Which file organization is used?**  
Sequential text-file organization. The program reads or writes records one by one, line by line.

**Q13. Why save data in files?**  
Without files, the in-memory records would disappear when the program closes. File storage allows the program to restore course and student records at the next start.

**Q14. What is the difference between completed courses and registration history?**  
Completed courses describe what the student has already completed. Registration history records the decisions the validator made for course requests. An accepted validation does not automatically mark a course as completed.

**Q15. What are the project's limitations?**  
The hash table supports up to 101 courses; deletion is not implemented; IDs/names cannot contain the `|` separator; and this is a validator rather than a full production enrollment system.

**Q16. What is the average and worst-case complexity of course lookup?**  
Average O(1) with a well-distributed, not-too-full hash table; worst-case O(N) if many collisions occur.

**Q17. Is a separate graph or DFS implemented?**  
No. Prerequisites are stored as linked lists attached to courses. A queue, a visited array, and loops traverse these links. The prerequisite relationship is graph-like in meaning, but there is no separate Graph class and no DFS.

## 14. How to divide work between teammates

- **Member 1 — Hashing and course catalogue:** explain `hashFunction()`, linear probing, course insertion/search, and course display.
- **Member 2 — Linked lists and student records:** explain the student list, completed-course list, and prerequisite list.
- **Member 3 — Validation and cycle prevention:** explain `collectMissingPrerequisites()`, `canReachCourse()`, queue operations, and the visited array.
- **Member 4 — File handling and testing:** explain sequential files, loading order, saving, the registration history, and test cases.

If the group has fewer members, combine the sections. Every member should know the overall flow: user request → find student/course → validate prerequisite data → find missing prerequisites → decide → append decision to the log.
