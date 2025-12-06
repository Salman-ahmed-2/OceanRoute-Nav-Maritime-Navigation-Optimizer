#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H
#include <cstring>
#include <utility>
#include <algorithm>
using namespace std;
class MyString
{ // String class
private:
    char *data;
    int length;

public:
    MyString() : data(nullptr), length(0) {}
    MyString(const char *str)
    {
        if (str)
        {
            length = strlen(str);
            data = new char[length + 1];
            strcpy(data, str);
        }
        else
        {
            data = nullptr;
            length = 0;
        }
    }
    MyString(const MyString &other)
    {
        length = other.length;
        if (length > 0)
        {
            data = new char[length + 1];
            strcpy(data, other.data);
        }
        else
        {
            data = nullptr;
        }
    }
    ~MyString()
    {
        delete[] data;
    }
    MyString &operator=(const MyString &other)
    {
        if (this != &other)
        {
            delete[] data;
            length = other.length;
            if (length > 0)
            {
                data = new char[length + 1];
                strcpy(data, other.data);
            }
            else
            {
                data = nullptr;
            }
        }
        return *this;
    }
    bool operator==(const MyString &other) const
    {
        if (length != other.length)
            return false;
        return strcmp(data, other.data) == 0;
    }
    bool operator==(const char *str) const
    {
        return strcmp(data, str) == 0;
    }
    const char *c_str() const { return data ? data : ""; }
    int size() const { return length; }
    bool empty() const { return length == 0; }
};
template <typename T>
class LinkedList
{ // Linked list
private:
    struct Node
    {
        T data;
        Node *next;
        Node(const T &val) : data(val), next(nullptr) {}
    };
    Node *head;
    Node *tail;
    int listSize;

public:
    LinkedList() : head(nullptr), tail(nullptr), listSize(0) {}
    ~LinkedList()
    {
        clear();
    }
    LinkedList(const LinkedList &other) : head(nullptr), tail(nullptr), listSize(0)
    {
        Node *curr = other.head;
        while (curr)
        {
            push_back(curr->data);
            curr = curr->next;
        }
    }
    LinkedList &operator=(const LinkedList &other)
    {
        if (this != &other)
        {
            clear();
            Node *curr = other.head;
            while (curr)
            {
                push_back(curr->data);
                curr = curr->next;
            }
        }
        return *this;
    }
    void push_back(const T &val)
    {
        Node *newNode = new Node(val);
        if (!head)
        {
            head = tail = newNode;
        }
        else
        {
            tail->next = newNode;
            tail = newNode;
        }
        listSize++;
    }
    void push_front(const T &val)
    {
        Node *newNode = new Node(val);
        newNode->next = head;
        head = newNode;
        if (!tail)
            tail = head;
        listSize++;
    }
    void insert(int index, const T &val)
    {
        if (index < 0 || index > listSize)
            return;
        if (index == 0)
        {
            push_front(val);
            return;
        }
        if (index == listSize)
        {
            push_back(val);
            return;
        }
        Node *current = head;
        for (int i = 0; i < index - 1; i++)
        {
            current = current->next;
        }
        Node *newNode = new Node(val);
        newNode->next = current->next;
        current->next = newNode;
        listSize++;
    }
    void remove(int index)
    {
        if (index < 0 || index >= listSize)
            return;
        if (index == 0)
        {
            Node *temp = head;
            head = head->next;
            delete temp;
            if (!head)
                tail = nullptr;
        }
        else
        {
            Node *current = head;
            for (int i = 0; i < index - 1; i++)
            {
                current = current->next;
            }
            Node *toDelete = current->next;
            current->next = toDelete->next;
            if (toDelete == tail)
                tail = current;
            delete toDelete;
        }
        listSize--;
    }
    T &get(int index)
    {
        static T defaultValue;
        if (index < 0 || index >= listSize)
            return defaultValue;
        Node *current = head;
        for (int i = 0; i < index; i++)
        {
            current = current->next;
        }
        return current->data;
    }
    const T &get(int index) const
    {
        static T defaultValue;
        if (index < 0 || index >= listSize)
            return defaultValue;
        Node *current = head;
        for (int i = 0; i < index; i++)
        {
            current = current->next;
        }
        return current->data;
    }
    void clear()
    {
        while (head)
        {
            Node *temp = head;
            head = head->next;
            delete temp;
        }
        head = tail = nullptr;
        listSize = 0;
    }
    int size() const { return listSize; }
    bool empty() const { return listSize == 0; }
    class Iterator
    {
    private:
        Node *current;

    public:
        Iterator(Node *node) : current(node) {}
        bool operator!=(const Iterator &other) const
        {
            return current != other.current;
        }
        Iterator &operator++()
        {
            if (current)
                current = current->next;
            return *this;
        }
        T &operator*() { return current->data; }
    };
    Iterator begin() { return Iterator(head); }
    Iterator end() { return Iterator(nullptr); }
};
template <typename T>
class Queue
{ // Queue class
private:
    struct Node
    {
        T data;
        Node *next;
        Node(const T &val) : data(val), next(nullptr) {}
    };
    Node *frontNode;
    Node *rearNode;
    int queueSize;

public:
    Queue() : frontNode(nullptr), rearNode(nullptr), queueSize(0) {}
    ~Queue()
    {
        while (!empty())
            dequeue();
    }
    Queue(const Queue &other) : frontNode(nullptr), rearNode(nullptr), queueSize(0)
    {
        Node *curr = other.frontNode;
        while (curr)
        {
            enqueue(curr->data);
            curr = curr->next;
        }
    }
    Queue &operator=(const Queue &other)
    {
        if (this != &other)
        {
            while (!empty())
                dequeue();
            Node *curr = other.frontNode;
            while (curr)
            {
                enqueue(curr->data);
                curr = curr->next;
            }
        }
        return *this;
    }
    void enqueue(const T &val)
    {
        Node *newNode = new Node(val);
        if (!rearNode)
        {
            frontNode = rearNode = newNode;
        }
        else
        {
            rearNode->next = newNode;
            rearNode = newNode;
        }
        queueSize++;
    }
    void dequeue()
    {
        if (frontNode)
        {
            Node *temp = frontNode;
            frontNode = frontNode->next;
            if (!frontNode)
                rearNode = nullptr;
            delete temp;
            queueSize--;
        }
    }
    T &front()
    {
        static T defaultValue;
        return frontNode ? frontNode->data : defaultValue;
    }
    const T &front() const
    {
        static T defaultValue;
        return frontNode ? frontNode->data : defaultValue;
    }
    bool empty() const { return frontNode == nullptr; }
    int size() const { return queueSize; }
};
template <typename T>
class Stack
{ // Stack class
private:
    struct Node
    {
        T data;
        Node *next;
        Node(const T &val) : data(val), next(nullptr) {}
    };
    Node *topNode;
    int stackSize;

public:
    Stack() : topNode(nullptr), stackSize(0) {}
    ~Stack()
    {
        while (!empty())
            pop();
    }
    Stack(const Stack &other) : topNode(nullptr), stackSize(0)
    {
        if (other.topNode)
        {
            topNode = new Node(other.topNode->data);
            Node *myCurr = topNode;
            Node *otherCurr = other.topNode->next;
            while (otherCurr)
            {
                myCurr->next = new Node(otherCurr->data);
                myCurr = myCurr->next;
                otherCurr = otherCurr->next;
            }
            stackSize = other.stackSize;
        }
    }
    Stack &operator=(const Stack &other)
    {
        if (this != &other)
        {
            while (!empty())
                pop();
            if (other.topNode)
            {
                topNode = new Node(other.topNode->data);
                Node *myCurr = topNode;
                Node *otherCurr = other.topNode->next;
                while (otherCurr)
                {
                    myCurr->next = new Node(otherCurr->data);
                    myCurr = myCurr->next;
                    otherCurr = otherCurr->next;
                }
                stackSize = other.stackSize;
            }
        }
        return *this;
    }
    void push(const T &val)
    {
        Node *newNode = new Node(val);
        newNode->next = topNode;
        topNode = newNode;
        stackSize++;
    }
    void pop()
    {
        if (topNode)
        {
            Node *temp = topNode;
            topNode = topNode->next;
            delete temp;
            stackSize--;
        }
    }
    T &top()
    {
        static T defaultValue;
        return topNode ? topNode->data : defaultValue;
    }
    const T &top() const
    {
        static T defaultValue;
        return topNode ? topNode->data : defaultValue;
    }
    bool empty() const { return topNode == nullptr; }
    int size() const { return stackSize; }
};
template <typename T>
class PriorityQueue
{ // Priority queue
private:
    struct HeapNode
    {
        T data;
        int priority;
        HeapNode(const T &d, int p) : data(d), priority(p) {}
    };
    HeapNode **heap;
    int capacity;
    int heapSize;
    void heapifyUp(int index)
    {
        while (index > 0)
        {
            int parent = (index - 1) / 2;
            if (heap[index]->priority >= heap[parent]->priority)
                break;
            swap(heap[index], heap[parent]);
            index = parent;
        }
    }
    void heapifyDown(int index)
    {
        while (true)
        {
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int smallest = index;
            if (left < heapSize && heap[left]->priority < heap[smallest]->priority)
                smallest = left;
            if (right < heapSize && heap[right]->priority < heap[smallest]->priority)
                smallest = right;
            if (smallest == index)
                break;
            swap(heap[index], heap[smallest]);
            index = smallest;
        }
    }

public:
    PriorityQueue(int cap = 1000) : capacity(cap), heapSize(0)
    {
        heap = new HeapNode *[capacity];
    }
    ~PriorityQueue()
    {
        for (int i = 0; i < heapSize; i++)
        {
            delete heap[i];
        }
        delete[] heap;
    }
    PriorityQueue(const PriorityQueue &other) : capacity(other.capacity), heapSize(other.heapSize)
    {
        heap = new HeapNode *[capacity];
        for (int i = 0; i < heapSize; i++)
        {
            heap[i] = new HeapNode(other.heap[i]->data, other.heap[i]->priority);
        }
    }
    PriorityQueue &operator=(const PriorityQueue &other)
    {
        if (this != &other)
        {
            for (int i = 0; i < heapSize; i++)
            {
                delete heap[i];
            }
            delete[] heap;
            capacity = other.capacity;
            heapSize = other.heapSize;
            heap = new HeapNode *[capacity];
            for (int i = 0; i < heapSize; i++)
            {
                heap[i] = new HeapNode(other.heap[i]->data, other.heap[i]->priority);
            }
        }
        return *this;
    }
    void push(const T &data, int priority)
    {
        if (heapSize >= capacity)
            return;
        heap[heapSize] = new HeapNode(data, priority);
        heapifyUp(heapSize);
        heapSize++;
    }
    T pop()
    {
        if (empty())
            return T();
        T result = heap[0]->data;
        delete heap[0];
        heap[0] = heap[--heapSize];
        heapifyDown(0);
        return result;
    }
    bool empty() const { return heapSize == 0; }
    int size() const { return heapSize; }
};
#endif