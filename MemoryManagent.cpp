#include <bits/stdc++.h>
#include <unistd.h>

using namespace std;

#define PROCESS_NAME_LEN 32 //进程名最大长度
#define MIN_SLICE 10 //内碎片最大大小
#define DEFAULT_MEM_SIZE 1024  //总内存大小
#define DEFAULT_MEM_START 0  //内存开始分配时的起始地址


typedef pair<int, string> My_algo;
int cnt = 0;

int mem_size = DEFAULT_MEM_SIZE;
bool flag = 0; //当内存以及被分配了之后，不允许更改总内存大小的flag
static int pid = 0;
My_algo algo[123];
int myAlgo = 1; //默认使用算法
struct free_block {
    free_block(int size, int startAddr, free_block *next) : size(size), start_addr(startAddr), next(next) {}

    //空闲数据块
    int size;
    int start_addr;
    struct free_block *next;
};

struct allocated_block { //已分配的数据块
    allocated_block(int size) : size(size) {}

    allocated_block(int pid, int size) : pid(pid), size(size) {}

    int pid;
    int size;
    int start_addr;
    int *data;
    struct allocated_block *next = nullptr;
};

free_block *free_block_head; //空闲数据块首指针
allocated_block *allocated_block_head = NULL; //分配块首指针

allocated_block *find_process(int id); //寻找pid为id的分配块
free_block *init_free_block(int mem_size); //空闲块初始化
void display_menu(); //显示选项菜单
void set_mem_size(); //设置内存大小
int allocate_mem(allocated_block *ab); //为制定块分配内存
void rearrange(); // 对块进行重新分配
int create_new_process(); //创建新的进程
int free_mem(allocated_block *ab); //释放分配块
void swap(int *p, int *q); //交换地址
int dispose(allocated_block *ab); //释放分配块结构体
void display_mem_usage(); //显示内存情况
void kill_process(); //杀死对应进程并释放其空间与结构体

void Usemy_algo(int id); //使用对应的分配算法
int first_fit_allocate_mem(allocated_block *ab);

int worst_fit_allocate_mem(allocated_block *ab);

int best_fit_allocate_mem(allocated_block *ab);

int buddy_system_allocate_mem(allocated_block *ab);

void add_to_allocate_block_list(allocated_block *ab);

int buddy_free_mem(allocated_block *ab);

struct buddy_controller {
    buddy_controller(int memSize) : mem_szie(memSize) {
        used_mem = 1;
        int i;
        for (i = 1; used_mem <= memSize; i++) {
            used_mem *= 2;
        }
        len = i - 1;
        used_mem = used_mem / 2;

        allocated_table = new bool[len];
        free_table = new bool[len];
        size_table = new int[len];
        int temp = 1;
        for (int j = 0; j < len; ++j) {
            size_table[j] = temp;
            temp *= 2;
        }
    }

    virtual ~buddy_controller() {
        delete[] allocated_table;
        delete[] free_table;
        delete[] size_table;
    }

    int len;
    int mem_szie;
    int used_mem;
    int *size_table;
    bool *allocated_table;
    bool *free_table;
};

buddy_controller *buddy_c = nullptr;

//主函数
int main() {

    int op;
    pid = 0;
    free_block_head = init_free_block(mem_size); //初始化一个可以使用的内存块，类似与操作系统可用的总存储空间
    for (;;) {
//        sleep(1);
        display_menu();
//        fflush(stdin);
        scanf("%d", &op);
        switch (op) {
            case 1: {
                set_mem_size();
                break;
            }
            case 2: {
                printf("Choose an algorithm\n");
                printf("1: Best Fit\n 2: Worst Fit\n 3: First Fit\n 4: Buddy System\n");
                int alg;
                scanf("%d", &alg);
                Usemy_algo(alg);
                break;
            }
            case 3: {
                if (myAlgo == 4 && buddy_c == nullptr) {
                    buddy_c = new buddy_controller(mem_size);
                    free_block_head->size = buddy_c->used_mem;//不是2的幂的部分内存buddy没法用
                }
                create_new_process();
                break;
            }
            case 4: {
                kill_process();
                break;
            }
            case 5: {
                display_mem_usage();
                break;
            }
            case 233: {
                delete buddy_c;
                puts("bye....");
                sleep(1);
                return 0;
            }
            defaut:
                break;
        }
    }
}

void Usemy_algo(int id) {
    myAlgo = id;
};

int first_fit_allocate_mem(allocated_block *ab) {
    int succ = 0;
    if (free_block_head == NULL) {
        return 0;
    }
    free_block *pre = nullptr;
    free_block *cur = free_block_head;
    while (cur != NULL) {
        if (cur->size = ab->size) {
            add_to_allocate_block_list(ab);
            if (cur == free_block_head) {
                free_block_head = free_block_head->next;
                ab->start_addr = cur->start_addr;
                free(cur);
                return 1;
            } else {
                pre->next = cur->next;
                ab->start_addr = cur->start_addr;
                free(cur);
                return 1;
            }
        } else if (cur->size > ab->size) {
            add_to_allocate_block_list(ab);
            ab->start_addr = cur->start_addr;
            cur->size = cur->size - ab->size;
            cur->start_addr = cur->start_addr + ab->size;
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}

int worst_fit_allocate_mem(allocated_block *ab) {
    free_block *ans = nullptr;
    free_block *pre_ans = nullptr;
    free_block *pre = nullptr;
    free_block *cur = free_block_head;
    while (cur != nullptr) {
        if ((ans == nullptr && cur->size >= ab->size) || (ans != nullptr && ans->size < cur->size)) {
            pre_ans = pre;
            ans = cur;
        }
        pre = cur;
        cur = cur->next;
    }
    if (ans == nullptr) {
        return 0;
    }
    add_to_allocate_block_list(ab);
    if (ans == free_block_head) {
        if (ans->size == ab->size) {
            ab->start_addr = ans->start_addr;
            free_block_head = free_block_head->next;
            free(ans);
        } else {
            ab->start_addr = ans->start_addr;
            ans->size = ans->size - ab->size;
            ans->start_addr = ans->start_addr + ab->size;
        }
    } else {
        //不是第一个空闲块
        if (ans->size == ab->size) {
            ab->start_addr = ans->start_addr;
            pre_ans->next = ans->next;
            free(ans);
        } else {
            ab->start_addr = ans->start_addr;
            ans->size = ans->size - ab->size;
            ans->start_addr = ans->start_addr + ab->size;
        }
    }
    return 1;
};

int best_fit_allocate_mem(allocated_block *ab) {
    free_block *ans = nullptr;
    free_block *pre_ans = nullptr;
    free_block *pre = nullptr;
    free_block *cur = free_block_head;
    while (cur != nullptr) {
        if ((ans == nullptr && cur->size >= ab->size) ||
            (ans != nullptr && ans->size > cur->size && cur->size >= ab->size)) {
            pre_ans = pre;
            ans = cur;
        }
        pre = cur;
        cur = cur->next;
    }
    if (ans == nullptr) {
        return 0;
    }
    add_to_allocate_block_list(ab);
    if (ans == free_block_head) {
        if (ans->size == ab->size) {
            ab->start_addr = ans->start_addr;
            free_block_head = free_block_head->next;
            free(ans);
        } else {
            ab->start_addr = ans->start_addr;
            ans->size = ans->size - ab->size;
            ans->start_addr = ans->start_addr + ab->size;
        }
    } else {
        //不是第一个空闲块
        if (ans->size == ab->size) {
            ab->start_addr = ans->start_addr;
            pre_ans->next = ans->next;
            free(ans);
        } else {
            ab->start_addr = ans->start_addr;
            ans->size = ans->size - ab->size;
            ans->start_addr = ans->start_addr + ab->size;
        }
    }
    return 1;
};

int buddy_system_allocate_mem(allocated_block *ab) {
    for (int i = 0; i < buddy_c->len; ++i) {
        if (buddy_c->size_table[i] >= ab->size) {//修改ab的大小为符合buddy算法的大小,即2的幂
            ab->size = buddy_c->size_table[i];
            break;
        }
    }
    free_block *cur = free_block_head;
    free_block *pre = nullptr;
    while (cur != nullptr && cur->size < ab->size) {
        pre = cur;
        cur = cur->next;
    }
    if (cur == nullptr) {
        return 0;
    }
    while (cur->size > ab->size) {

        cur->next = new free_block(cur->size / 2, cur->start_addr + (cur->size / 2), cur->next);
        cur->size = cur->size / 2;
    }
    if (pre != nullptr) {
        pre->next = cur->next;
    } else {
        free_block_head = cur->next;
    }
    ab->start_addr = cur->start_addr;
    add_to_allocate_block_list(ab);
    return 1;
};

free_block *find_pre_free(free_block *fb) {
    free_block *cur = free_block_head;
    free_block *pre = nullptr;
    while (cur != fb) {
        pre = cur;
        cur = cur->next;
    }
    return pre;
}

/**
 * 在分配存储块时经常会将一个大的存储块分裂成两个大小相等的小块,
 * 那么这两个小块就称为"伙伴".在Buddy System进行合并时,
 * 只会将互为伙伴的两个存储块合并成大块, 也就是说如果有两个存储块大小相同, 地址也相邻,
 * 但是不是由同一个大块分裂出来的, 也不会被合并起来.
 * 正常情况下, 起始地址为p, 大小为Pow(2, k)的存储块,
 * 其伙伴块的起始地址为: p + Pow(2, k) 和 p - Pow(2, k).
 * @param ab
 * @return
 * test case: 2 4 3 1 3 6 3 44 3 88 3 5 3 99 5 然后再一个个用4+空格+pid kill .
 */
int buddy_free_mem(allocated_block *ab) {
    free_block *cur = free_block_head;
    free_block *pre = nullptr;
    while (cur != nullptr && cur->start_addr < ab->start_addr) {
        pre = cur;
        cur = cur->next;
    }
    int update;
    free_block *mid = new free_block(ab->size, ab->start_addr, nullptr);
    while (1) {
        update = 0;
        /**
         * pre->start_addr%(pre->size*2)==0  两个都是用来验证两个块是buddy(同一个大块分裂而成的)关系,画图可以理解.
         * (mid->start_addr%(mid->size*2)==0)
         *
         */
        if (pre != nullptr && (pre->size == mid->size) && (pre->start_addr + pre->size == mid->start_addr) &&
            (pre->start_addr % (pre->size * 2) == 0)) {
            pre->size *= 2;
            delete mid;
            mid = pre;
            pre = find_pre_free(pre);
            update++;
        } else if (cur != nullptr && (cur->size == mid->size) && (mid->start_addr + mid->size == cur->start_addr) &&
                   (mid->start_addr % (mid->size * 2) == 0)) {
            mid->size *= 2;
            cur = cur->next;
            update++;
        }
        if (!update) {
            break;
        }
    }
    if (pre != nullptr) {
        pre->next = mid;
    } else {
        free_block_head = mid;
    }
    mid->next = cur;
    return 1;
}

void add_to_allocate_block_list(allocated_block *ab) {
    if (allocated_block_head == NULL) {
        allocated_block_head = ab;
        return;
    }
    allocated_block *cur = allocated_block_head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = ab;
}

allocated_block *find_process(int id) { //循环遍历分配块链表，寻找pid=id的进程所对应的块
    allocated_block *cur = allocated_block_head;
    while (cur != nullptr) {
        if (cur->pid == id) {
            return cur;
        }
        cur = cur->next;
    }
    return nullptr;
}

free_block *init_free_block(int mem_size) { //初始化空闲块，这里的mem_size表示允许的最大虚拟内存大小
    free_block *p;
    p = (free_block *) malloc(sizeof(free_block));
    if (p == NULL) {
        puts("No memory left");
        return NULL;
    }
    p->size = mem_size;
    p->start_addr = DEFAULT_MEM_START;
    p->next = NULL;
    return p;
}

void display_menu() {
    puts("\n\n******************menu*******************");
    printf("1) Set memory size (default = %d)\n", DEFAULT_MEM_SIZE);
    printf("2) Set memory allocation algorithm\n");
    printf("3) Create a new process\n");
    printf("4) Kill a process\n");
    printf("5) Display memory usage\n");
    printf("233) Exit\n");
}

void set_mem_size() { //更改最大内存大小
    if (flag == 1) {
        printf("cant change the max mem size after creating process!");
        return;
    }
    printf("Please input memory size\n");
    scanf("%d", &mem_size);
    free_block_head->size = mem_size;

}

int allocate_mem(allocated_block *ab) { //为块分配内存，真正的操作系统会在这里进行置换等操作
    switch (myAlgo) {
        case 1: {
            return best_fit_allocate_mem(ab);
        }
        case 2: {
            return worst_fit_allocate_mem(ab);

        }
        case 3: {
            return first_fit_allocate_mem(ab);
        }
        case 4: {
            return buddy_system_allocate_mem(ab);
        }
    }
}

int create_new_process() { //创建新进程
    flag = 1;
    int mem_sz = 0;
    printf("Please input memory size\n");
    scanf("%d", &mem_sz);
    // Write your code here
    allocated_block *ab = new allocated_block(pid++, mem_sz);
    int succ = allocate_mem(ab);
    if (!succ) {
        pid--;
        delete ab;
        printf("No enough memory \n");
        return -1;
    }
    return pid - 1;
}

void swap(int *p, int *q) {
    int tmp = *p;
    *p = *q;
    *q = tmp;
    return;
}

void rearrange() { //将块按照地址大小进行排序
    free_block *tmp, *tmpx;
    puts("Rearrange begins...");
    puts("Rearrange by address...");
    tmp = free_block_head;
    while (tmp != NULL) {
        tmpx = tmp->next;
        while (tmpx != NULL) {
            if (tmpx->start_addr < tmp->start_addr) {
                swap(&tmp->start_addr, &tmpx->start_addr);
                swap(&tmp->size, &tmpx->size);
            }
            tmpx = tmpx->next;
        }
        tmp = tmp->next;
    }
    usleep(500);
    puts("Rearrange Done.");
}


int free_mem(allocated_block *ab) { //释放某一块的内存
    free_block *pre, *cur;
    if (ab->start_addr == DEFAULT_MEM_START) {
        if (free_block_head == nullptr) {
            free_block_head = new free_block(ab->size, DEFAULT_MEM_START, nullptr);
        } else {
            if (ab->start_addr + ab->size == free_block_head->start_addr) {
                free_block_head->start_addr = DEFAULT_MEM_START;
                free_block_head->size = free_block_head->size + ab->size;
            } else {
                cur = new free_block(ab->size, DEFAULT_MEM_START, free_block_head);
                free_block_head = cur;
            }
        }
    } else {
        if (free_block_head == nullptr) {
            free_block_head = new free_block(ab->size, ab->size, nullptr);
        } else if (free_block_head->start_addr < ab->start_addr) {
            if (free_block_head->start_addr + free_block_head->size == ab->start_addr) {//两个free内存合并
                free_block_head->size += ab->size;
            } else {
                pre = free_block_head;
                cur = pre->next;
                while (cur != nullptr && cur->start_addr < ab->start_addr) {
                    pre = cur;
                    cur = cur->next;
                }
                if (cur == nullptr) {
                    if (pre->start_addr + pre->size == ab->start_addr) {
                        pre->size += ab->size;
                    } else {//不能合并
                        pre->next = new free_block(ab->size, ab->start_addr, nullptr);
                    }
                } else {//cur不是nullptr
                    if ((pre->start_addr + pre->size == ab->start_addr) &&
                        (ab->start_addr + ab->size == cur->start_addr)) {//前中后都能合并
                        pre->next = pre->next->next;
                        pre->size += ab->size + cur->size;
                    } else if (pre->start_addr + pre->size == ab->start_addr) {//前中能合并
                        pre->size += ab->size;
                    } else if (ab->start_addr + ab->size == cur->start_addr) {//中后能合并
                        cur->start_addr -= ab->size;
                    } else {//都不能合并
                        pre->next = new free_block(ab->size, ab->start_addr, nullptr);
                        pre->next->next = cur;
                    }
                }
            }
        } else {//startaddr: head大于ab
            if (ab->start_addr + ab->size == free_block_head->start_addr) {
                free_block_head->start_addr -= ab->size;
                free_block_head->size += ab->size;
            } else {
                free_block_head = new free_block(ab->size, ab->start_addr, free_block_head);
            }
        }

    }
}


int dispose(allocated_block *fab) { //释放结构体所占的内存
    allocated_block *pre, *ab;
    if (fab == allocated_block_head) {
        allocated_block_head = allocated_block_head->next;
        free(fab);
        return 1;
    }
    pre = allocated_block_head;
    ab = allocated_block_head->next;
    while (ab != fab) {
        pre = ab;
        ab = ab->next;
    }
    pre->next = ab->next;
    free(ab);
    return 2;
}

void display_mem_usage() {
    free_block *fb = free_block_head;
    allocated_block *ab = allocated_block_head;
    puts("*********************Free Memory*********************");
    printf("%20s %20s\n", "start_addr", "size");
    int cnt = 0;
    while (fb != NULL) {
        cnt++;
        printf("%20d %20d\n", fb->start_addr, fb->size);
        fb = fb->next;
    }
    if (!cnt) puts("No Free Memory");
    else printf("Totaly %d free blocks\n", cnt);
    puts("");
    puts("*******************Used Memory*********************");
    printf("%10s %10s %20s\n", "PID", "start_addr", "size");
    cnt = 0;
    while (ab != NULL) {
        cnt++;
        printf("%10d %10d %20d\n", ab->pid, ab->start_addr, ab->size);
        ab = ab->next;
    }
    if (!cnt) puts("No allocated block");
    else printf("Totaly %d allocated blocks\n", cnt);
    return;
}

void kill_process() { //杀死某个进程
    allocated_block *ab;
    int pid;
    puts("Please input the pid of Killed process");
    scanf("%d", &pid);

    ab = find_process(pid);
    if (ab != NULL) {
        if (myAlgo != 4) {
            free_mem(ab);
        } else {
            buddy_free_mem(ab);
        }
        dispose(ab);
    } else {
        printf("no this pid!");
    }
}



