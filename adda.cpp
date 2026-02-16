#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <string>
#include <limits>
#include <climits>   

using namespace std;

class Date {
private:
    int day, month, year;
    bool isLeapYear(int y) const;
    bool isValidDate(int d, int m, int y) const;

public:
    Date();
    Date(int d, int m, int y);
    static Date today();

    bool operator<(const Date& other) const;
    bool operator<=(const Date& other) const;
    friend ostream& operator<<(ostream& out, const Date& d);
    friend istream& operator>>(istream& in, Date& d);
};

Date::Date() : day(1), month(1), year(2000) {}

Date::Date(int d, int m, int y) {
    if (isValidDate(d, m, y)) {
        day = d; month = m; year = y;
    } else {
        day = 1; month = 1; year = 2000;
    }
}

bool Date::isLeapYear(int y) const {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

bool Date::isValidDate(int d, int m, int y) const {
    if (y < 0 || m < 1 || m > 12 || d < 1) return false;
    int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 2 && isLeapYear(y))
        return d <= 29;
    return d <= daysInMonth[m-1];
}

Date Date::today() {
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    return Date(now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);
}

bool Date::operator<(const Date& other) const {
    if (year != other.year) return year < other.year;
    if (month != other.month) return month < other.month;
    return day < other.day;
}

bool Date::operator<=(const Date& other) const {
    return !(other < *this);
}

ostream& operator<<(ostream& out, const Date& d) {
    out << d.day << '/' << d.month << '/' << d.year;
    return out;
}

istream& operator>>(istream& in, Date& d) {
    int dd, mm, yy;
    in >> dd >> mm >> yy;
    d = Date(dd, mm, yy);
    return in;
}

class Priority {
private:
    int value;
    
public:
    Priority() : value(1) {}
    Priority(int v) {
        if (v >= 1 && v <= 3)
            value = v;
        else
            value = 1;
    }
    int getValue() const { return value; }
    
    string getName() const {
        switch(value) {
            case 1: return "Low";
            case 2: return "Medium";
            case 3: return "High";
            default: return "Unknown";
        }
    }
    
    bool operator==(const Priority& other) const {
        return value == other.value;
    }
    
    bool operator!=(const Priority& other) const {
        return value != other.value;
    }
    
    bool operator>(const Priority& other) const {
        return value > other.value;
    }
    
    bool operator<(const Priority& other) const {
        return value < other.value;
    }
    friend ostream& operator<<(ostream& out, const Priority& p) {
        out << p.value;
        return out;
    }
    
    friend istream& operator>>(istream& in, Priority& p) {
        int v;
        in >> v;
        p = Priority(v);
        return in;
    }
};

class Task {
protected:
    string title;
    string description;
    bool done;
    Priority priority;
    Date creationDate;
    Date deadline;

public:
    Task(string t, string d, Priority p, Date cd, Date dl, bool status = false);
    virtual ~Task() {}

    void markDone();
    bool isDone() const;
    Priority getPriority() const;
    Date getDeadline() const;

    virtual bool isOverdue() const;
    virtual void display() const;
    virtual void save(ofstream& out) const;
};

Task::Task(string t, string d, Priority p, Date cd, Date dl, bool status)
    : title(t), description(d), priority(p), creationDate(cd), deadline(dl), done(status) {}

void Task::markDone() { done = true; }

bool Task::isDone() const { return done; }

Priority Task::getPriority() const { return priority; }

Date Task::getDeadline() const { return deadline; }

bool Task::isOverdue() const {
    return !done && deadline < Date::today();
}

void Task::display() const {
    cout << "Title       : " << title << endl;
    cout << "Description : " << description << endl;
    cout << "Priority    : " << priority.getName() << endl;
    cout << "Status      : " << (done ? "Done" : "Pending") << endl;
    cout << "Created     : " << creationDate << endl;
    cout << "Deadline    : " << deadline << endl;
}

void Task::save(ofstream& out) const {
    out << "0\n";   
    out << title << "\n" << description << "\n";
    out << priority << " " << done << "\n";
    out << creationDate << "\n";
    out << deadline << "\n";
}

class DeadlineTask : public Task {
public:
    DeadlineTask(string t, string d, Priority p, Date cd, Date dl, bool status = false);
    void display() const override;
    void save(ofstream& out) const override;
};

DeadlineTask::DeadlineTask(string t, string d, Priority p, Date cd, Date dl, bool status)
    : Task(t, d, p, cd, dl, status) {}

void DeadlineTask::display() const {
    cout << "[Deadline Task - Special]" << endl;
    Task::display();
}

void DeadlineTask::save(ofstream& out) const {
    out << "1\n";   
    out << title << "\n" << description << "\n";
    out << priority << " " << done << "\n";
    out << creationDate << "\n";
    out << deadline << "\n";
}

class TodoList {
private:
    vector<Task*> tasks;

public:
    ~TodoList();
    void addTask(Task* t);
    void showAll() const;
    void showByStatus(bool status) const;   
    void showOverdue() const;
    void markDone(int index);
    void removeTask(int index);
    void sortByPriority();
    void sortByDeadline();

    void saveToFile() const;
    void loadFromFile();
};

TodoList::~TodoList() {
    saveToFile();
    for (Task* t : tasks)
        delete t;
}

void TodoList::addTask(Task* t) {
    tasks.push_back(t);
}

void TodoList::showAll() const {
    if (tasks.empty()) {
        cout << "\nNo tasks to display.\n";
        return;
    }
    for (size_t i = 0; i < tasks.size(); ++i) {
        cout << "\n" << string(40, '-') << "\n";
        cout << "  TASK #" << i+1 << "\n";
        cout << string(40, '-') << "\n";
        tasks[i]->display();
    }
}

void TodoList::showByStatus(bool status) const {
    bool found = false;
    for (size_t i = 0; i < tasks.size(); ++i) {
        if (tasks[i]->isDone() == status) {
            if (!found) {
                cout << "\n" << string(40, '-') << "\n";
                cout << (status ? "COMPLETED TASKS" : "PENDING TASKS") << "\n";
                cout << string(40, '-') << "\n";
                found = true;
            }
            cout << "\nTask #" << i+1 << ":\n";
            tasks[i]->display();
        }
    }
    if (!found) {
        cout << "\nNo " << (status ? "completed" : "pending") << " tasks.\n";
    }
}

void TodoList::showOverdue() const {
    bool found = false;
    for (size_t i = 0; i < tasks.size(); ++i) {
        if (tasks[i]->isOverdue()) {
            if (!found) {
                cout << "\n" << string(40, '-') << "\n";
                cout << "OVERDUE TASKS\n";
                cout << string(40, '-') << "\n";
                found = true;
            }
            cout << "\nTask #" << i+1 << ":\n";
            tasks[i]->display();
        }
    }
    if (!found) {
        cout << "\nNo overdue tasks. Good job!\n";
    }
}

void TodoList::markDone(int index) {
    if (index < 0 || index >= (int)tasks.size())
        throw out_of_range("Invalid task number.");
    tasks[index]->markDone();
    cout << "Task marked as done.\n";
}

void TodoList::removeTask(int index) {
    if (index < 0 || index >= (int)tasks.size())
        throw out_of_range("Invalid task number.");
    delete tasks[index];
    tasks.erase(tasks.begin() + index);
    cout << "Task removed.\n";
}

void TodoList::sortByPriority() {
    sort(tasks.begin(), tasks.end(),
         [](Task* a, Task* b) { return a->getPriority() > b->getPriority(); });
    cout << "Tasks sorted by priority (High to Low).\n";
}

void TodoList::sortByDeadline() {
    sort(tasks.begin(), tasks.end(),
         [](Task* a, Task* b) { return a->getDeadline() < b->getDeadline(); });
    cout << "Tasks sorted by deadline (earliest first).\n";
}

void TodoList::saveToFile() const {
    ofstream out("tasks.txt");
    if (!out) {
        cerr << "Error: Could not save tasks to file.\n";
        return;
    }
    for (Task* t : tasks)
        t->save(out);
}

void TodoList::loadFromFile() {
    ifstream in("tasks.txt");
    if (!in) return;  

    while (true) {
        int type;
        in >> type;
        if (!in) break;
        in.ignore();

        string title, desc;
        getline(in, title);
        getline(in, desc);

        int p, done;
        in >> p >> done;
        in.ignore();

        Date creation;
        in >> creation;

        Date deadline;
        in >> deadline;

        Priority priority(p);

        if (type == 0) {
            tasks.push_back(new Task(title, desc, priority, creation, deadline, done));
        } else {
            tasks.push_back(new DeadlineTask(title, desc, priority, creation, deadline, done));
        }
    }
}

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int getIntInput(const string& prompt, int min, int max) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (cin.fail() || value < min || value > max) {
            cout << "Invalid input. Please enter a number between " << min << " and " << max << ".\n";
            clearInput();
        } else {
            clearInput();
            return value;
        }
    }
}

int main() {
    TodoList todo;
    todo.loadFromFile();

    int choice;
    do {
        cout << "\n" << string(50, '=') << "\n";
        cout << "            TODO LIST MANAGER\n";
        cout << string(50, '=') << "\n";
        cout << "  1. Add new task\n";
        cout << "  2. Show all tasks\n";
        cout << "  3. Show completed tasks\n";
        cout << "  4. Show pending tasks\n";
        cout << "  5. Show overdue tasks\n";
        cout << "  6. Mark task as done\n";
        cout << "  7. Delete a task\n";
        cout << "  8. Sort by priority\n";
        cout << "  9. Sort by deadline\n";
        cout << "  0. Exit\n";
        cout << string(50, '-') << "\n";
        choice = getIntInput("  Your choice: ", 0, 9);

        try {
            switch (choice) {
                case 1: {
                    string title, desc;
                    cout << "\n--- New Task ---\n";
                    cout << "Title: ";
                    getline(cin, title);
                    cout << "Description: ";
                    getline(cin, desc);

                    int p = getIntInput("Priority (1=Low, 2=Medium, 3=High): ", 1, 3);
                    Priority priority(p);

                    Date creation = Date::today();
                    cout << "Creation date set to today: " << creation << "\n";

                    cout << "Enter deadline (day month year, e.g. 15 8 2025): ";
                    Date deadline;
                    cin >> deadline;
                    clearInput();

                    int type = getIntInput("Task type (1=Normal, 2=Special DeadlineTask): ", 1, 2);

                    if (type == 1) {
                        todo.addTask(new Task(title, desc, priority, creation, deadline));
                    } else {
                        todo.addTask(new DeadlineTask(title, desc, priority, creation, deadline));
                    }
                    cout << "Task added successfully.\n";
                    break;
                }

                case 2:
                    todo.showAll();
                    break;

                case 3:
                    todo.showByStatus(true);
                    break;

                case 4:
                    todo.showByStatus(false);
                    break;

                case 5:
                    todo.showOverdue();
                    break;

                case 6: {
                    int index = getIntInput("Enter task number to mark as done: ", 1, INT_MAX);
                    todo.markDone(index - 1);
                    break;
                }

                case 7: {
                    int index = getIntInput("Enter task number to delete: ", 1, INT_MAX);
                    cout << "Are you sure? (y/n): ";
                    char confirm;
                    cin >> confirm;
                    clearInput();
                    if (confirm == 'y' || confirm == 'Y') {
                        todo.removeTask(index - 1);
                    } else {
                        cout << "Deletion cancelled.\n";
                    }
                    break;
                }

                case 8:
                    todo.sortByPriority();
                    break;

                case 9:
                    todo.sortByDeadline();
                    break;

                case 0:
                    cout << "Goodbye! Your tasks have been saved.\n";
                    break;
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    } while (choice != 0);

    return 0;
}