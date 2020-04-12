#include <iostream>
#include <cstring>

using namespace std;

/*
 Banker’s Algorithm
 First line is an integer r, which is the number of resource types.
 The second line will be r integers, which are the maximum quantity of each resource.
  Then will be following a list of commands. The commands are in three form:
 New process registering, such as “1678 new 6 5 0 7”, means
 process 1678 is a new process, whose maximum need of each resource is 6 5 0 7 (assume r is 4)

 Resource requesting, such as “233 request 0 4 5 3”, means
 process 233 is an old process, it request more resource, the need of each resource is 0 4 5 3.

 Process termination, such as “233 terminate”, means
 process 233 terminate and return all resources it holding.

Output:
For each command, output “OK” or “NOT OK” to determine if the command can execute. If OK, execute the command.

 input:
3
4 2 3
233 new 1 2 3
888 new 4 3 3
777 new 4 2 3
233 request 1 2 0
777 request 0 0 4
777 request 0 0 3
233 terminate

output:


OK
NOT OK
OK
OK
NOT OK
NOT OK
OK

 */

template<class T>
class node {
public:
    node() = default;

    node(int id, T *arr, int rescSize) {
        this->rescSize = rescSize;
        this->id = id;
        this->rescNeedNow = arr;
        rescNeedAtBegin = (T *) malloc(sizeof(T) * rescSize);
//        memset(rescNeedAtBegin, 0, sizeof(T) * rescSize);
        for (int i = 0; i < rescSize; ++i) {
            *(rescNeedAtBegin + i) = *(rescNeedNow + i);
        }
    }

    void printNodeMsg() {
        cout << "\nid: " << id << "\nrescNeedNow:\trescNeedAtBegin:\n";
        for (int i = 0; i < rescSize; ++i) {
            cout << *(rescNeedNow + i) << "\t" << *(rescNeedAtBegin + i) << endl;
        }
    }

    int rescSize;
    int id;
    T *rescNeedNow = nullptr;
    T *rescNeedAtBegin = nullptr;
    node<T> *next;
    node<T> *prev;
};

template<class E>
class LinkedList {
public:
    LinkedList(int resourceSize) {
        this->rescSize = resourceSize;
        this->size = 0;
        this->head = new node<E>();
        this->tail = new node<E>();
        this->head->next = this->tail;
        this->tail->prev = this->head;
        this->curr = this->head;
    }

    bool giveResource(node<E> *node, E *resource) {
        for (int i = 0; i < rescSize; ++i) {
            if (node->rescNeedNow[i] < resource[i]) {
//                cout << "request much than the node need\n";
                return false;
            } else if (rescAvilNow[i] < resource[i]) {
//                cout << "request much than the resource available\n";
                return false;
            }
        }
        for (int j = 0; j < rescSize; ++j) {
//            node->rescNeedNow[j] -= resource[j];
//            rescAvilNow[j] -= resource[j];
            node->rescNeedNow[j] = node->rescNeedNow[j] - resource[j];
            rescAvilNow[j] = rescAvilNow[j] - resource[j];

        }
        return true;
    }

    bool getResourceBack(node<E> *node, E *resource) {
        for (int i = 0; i < rescSize; ++i) {
            resource[i] = -1 * resource[i];
        }
        giveResource(node, resource);
        //change the resource bake
        for (int i = 0; i < rescSize; ++i) {
            resource[i] = -1 * resource[i];
        }
        return true;
    }

    bool wilGoToDeadLock() {
        bool flag = true;
        curr = head->next;
        for (int i = 0; i < size; ++i) {
            flag = false;
            for (int j = 0; j < rescSize; ++j) {
                if (rescAvilNow[j] < curr->rescNeedNow[j]) {
                    flag = true;
                    break;
                }
            }
            if (!flag) {
                return false;
            }
            curr = curr->next;
        }
        return true;
    }

    int searchId(int id) {
        curr = head;
        for (int i = 0; i < size; ++i) {
            curr = curr->next;
            if (curr->id == id) {
                return 1;
            }
        }
        return 0;
    }

    int searchId(int id, node<E> **node) {
        curr = head;
        for (int i = 0; i < size; ++i) {
            curr = curr->next;
            if (curr->id == id) {
                *node = curr;
                return 1;
            }
        }
        return 0;
    }

    void add(node<E> *position, int id, E *arr) {
        node<E> *temp = new node<E>(id, arr, rescSize);
        temp->next = position->next;
        temp->prev = position;
        temp->prev->next = temp;
        temp->next->prev = temp;
        size++;
    }

    void addToTail(int id, E *arr) {
        add(tail->prev, id, arr);
    }

    void removeAndGetRescBack(node<E> *nd) {
        if (size == 0) {
//            cout << "cant remove from empty list" << endl;
        } else if (nd == head || nd == tail) {
//            cout << "cant remove default head or tail" << endl;
        }
        for (int i = 0; i < rescSize; ++i) {
            rescAvilNow[i] = rescAvilNow[i] + nd->rescNeedAtBegin[i] - nd->rescNeedNow[i];
        }
        nd->prev->next = nd->next;
        nd->next->prev = nd->prev;
        delete nd;
        size--;
    }

    void removeAndGetRescBack(int id) {
        node<E> *nd;
        searchId(id, &nd);
        removeAndGetRescBack(nd);
    }

    E *rescAvilNow = nullptr;
    E *rescAtBegin = nullptr;
    int size;
    int rescSize;
    //resource max store in head->arr
    node<E> *head;
    //resource avil store in tail->arr
    node<E> *tail;
    node<E> *curr;
};

int main() {
    string ok = "OK";
    string nok = "NOT OK";
    bool fine = true;
    int n = 0;
    cin >> n;
    LinkedList<int> *list = new LinkedList<int>(n);
    int Max[n];
    int Avil[n];
    for (int i = 0; i < n; ++i) {
        cin >> Max[i];
        Avil[i] = Max[i];
    }
    list->rescAtBegin = Max;
    list->rescAvilNow = Avil;
    int id = 4;
    string msg = "fdfdfdf";
    cin >> id;
    while (1) {
        cin >> msg;
        int *resc = (int *) malloc(sizeof(int) * n);
        memset(resc, 0, sizeof(int) * n);
//     int* resc = new int[n];
        if (msg == "terminate") {
            list->removeAndGetRescBack(id);
            cout << ok << endl;
            continue;
        }
        for (int i = 0; i < n; ++i) {
            cin >> resc[i];
        }
        if (msg == "new") {
            fine = true;
            for (int i = 0; i < n; ++i) {
                if (resc[i] > *((list->rescAtBegin) + i)) {
                    fine = false;
                    break;
                }
            }
            if (fine) {
                if (list->searchId(id)) {
                    fine = false;
                }
            }
            if (fine) {
                list->addToTail(id, resc);

            } else
                fine = false;
        } else if (msg == "request") {
            //here need node** because i need to give address to a address.
            node<int> *node = nullptr;
            list->searchId(id, &node);
            fine = list->giveResource(node, resc);
            if (fine) {
                if (list->wilGoToDeadLock()) {
                    list->getResourceBack(node, resc);
                    fine = false;

                } else {

                }
            }

//            node->printNodeMsg();
        }
        if (fine) {
            cout << ok << endl;
        } else {
            cout << nok << endl;
        }
        if (!(cin >> id)) {
            break;
        }
    }
    return 0;
}
