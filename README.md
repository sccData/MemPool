# MemPool
学习内存池

#### 内存分配基础

![1](.\1.PNG)

| 分配                       | 释放                         | 类别      | 可否重载                 |
| -------------------------- | ---------------------------- | --------- | ------------------------ |
| malloc()                   | free()                       | C函数     | 不可                     |
| new                        | delete                       | C++表达式 | 不可                     |
| ::operator new()           | ::operator delete()          | C++函数   | 可                       |
| allocator\<T\>::allocate() | allocator\<T\>::deallocate() | C++标准库 | 可自由设计并搭配任何容器 |

```c++
// 1.
void* p1 = malloc(512);
free(p1);

// 2.
complex<int>* p2 = new complex<int>;
delete p2;

// 3.
void* p3 = ::operator new(512);
::operator delete(p3);

// 4. GNUC2.9
void* p4 = alloc::allocate(512);
alloc::deallocate(p4, 512);

// 5. GNUC4.9
void* p4 = allocator<int>().allocate(7);
allocator<int>().deallocate((int*)p4, 7);

void* p5 = __gnu_cxx::__pool_alloc<int>().allocate(9);
__gnu_cxx::__pool_alloc<int>().deallocate((int*)p5, 9);
```

#### new expression

```c++
Complex* pc = new Complex(1, 2);
```

编码器转为(与实际有区别, 仅用于理解)

```c++
Complex* pc;
try {
    void* mem = operator new(sizeof(Complex));
    pc = static_cast<Complex*>(mem);
    pc->Complex::Complex(1, 2);
    // 注意: 只有编译器可以这样使用
} catch(std::bad_alloc) {
    
}
```

operator new在底层调用malloc, 本质上是对malloc进行封装

```c++
delete pc;
```

编码器转为(与实际有区别, 仅用于理解)

```c++
pc->~Complex();
operator delete(pc);
```

operator delete在底层调用free, 本质上是对free进行封装

#### array new, array delete

```c++
// 换起三次构造函数
Complex* pca = new Complex[3];

// 换起三次析构函数
delete[] pca;
```

对于delete, 如果去掉[], 对于class without ptr member来说, 可能没有影响, 因为cookie记录内存分配的大小; 但是对于class with pointer member来说, 通常有影响, 因为cookie没有记录需要进行多少次析构, 所以可能会发生内存泄漏.

![2](.\2.PNG)

#### placement new

placement new允许我们将object构建与allocated memory中

```c++
#include <new>
char* buf = new char[sizeof(Complex) * 3];
Complex* pc = new(buf) Complex(1, 2);
```

编码器转为(与实际有区别, 仅用于理解)

```c++
Complex* pc;
try {
    void* mem = operator new(sizeof(Complex), buf);
    pc = static_cast<Complex*>(mem);
    pc->Complex::Complex(1, 2);
} catch(std::bad_alloc) {
    
}
```

``` c++
void* operator new(size_t, void* loc) {
    return loc;    // 无任何行为
}
```

**注意, 关于"placement new"或指new(p), 或指::operator new(size, void*)**

#### 重载::operator new/::operator delete

```c++
void* myAlloc(size_t size)
{ return malloc(size); }

void myFree(void* ptr)
{ return free(ptr); }

inline void* operator new(size_t size) {
    cout << "jjhou global new() \n";
    return myAlloc(size);
}

inline void* operator new[](size_t size) {
    cout << "jjhou global new[]() \n";
    return myAlloc(size);
}

inline void operator delete(void* ptr) {
    cout << "jjhou global delete() \n";
    return myFree(ptr);
}

inline void operator delete(void* ptr) {
    cout << "jjhou global delete[] \n";
    myFree(ptr);
}
```

#### 重载类内operator new和operator delete

![3](.\3.PNG)

![4](.\4.PNG)

#### 重载new()/delete()

可以重载class member operator new(), 写出多个版本, 前提是每一个版本的声明都必须有独特的参数列, 其中第一参数必须是size_t, 其余参数以new所指定的placement arguments为初值. 出现于new(...)小括号内的便是所谓placement arguments.

```c++
Foo* pf = new(300, 'c') Foo;
```

我们也可以重载class member operator delete(), 写出多个版本. 但是它们绝不会被delete调用. 只有当new所调用的ctor抛出exception, 才会调用这些重载版本的operator delete(). 它只可能这样被调用, 主要用来归还未能完全创建成功地object所占用的memory.

**在G4.9没有调用, 而G2.9调用**

#### new handler

当operator new没能力为你分配出所申请的memory, 会抛出一个std::bad_alloc exception. 某些老牌编译器则是返回0——你仍然可以令编译器那么做:

```c++ 
new (nothrow) Foo;
```

抛出exception之前会先(不知一次)调用一个可由client指定的handler.

```c++
typedef void(*new_handler)();
new_handler set_new_handler(new_handler p) throw();
```

设计良好的new_handler只有两种选择:

* 让更多memory可用
* 调用abort()或exit()

#### global allocator

在使用内存池的分配器中, 维护了16个free lists来避免小额区块造成内存碎片, 小额区块存在下列问题:

* 产生内存碎片
* 额外负担. 额外负担是一些区块信息, 用于管理内存. 区块越小, 额外负担所占的比例就越大, 越显浪费

每次分配一大块内存, 并维护对应的自由链表(free-list), 下次若载有相同大小的内存需求, 就直接从free-list中拨出. 如果客户释放小额区块, 就由分配器回收到free-list中. **维护有16个free-list**，各自管理大小分别为8，16，24，32，40，48，56，64，72，80，88，96，104，112，120，128bytes的小额区块.

![5](.\5.PNG)

![6](.\6.PNG)

![7](.\7.PNG)

![8](.\8.PNG)

![9](.\9.PNG)

![10](.\10.PNG)

![11](.\11.PNG)

![12](.\12.PNG)

![13](.\13.PNG)

![14](.\14.PNG)

![15](.\15.PNG)

![16](.\16.PNG)

