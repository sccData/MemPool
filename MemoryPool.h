#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include <cstddef>
class MemoryPool {
private:
	const size_t ALIGN = 8;
	const size_t MAXBYTES = 128;
	const size_t NFREELISTS = MAXBYTES / CALIGN;

private:
	static size_t ROUND_UP(size_t bytes) {
		return (((bytes) + ALIGN - 1) & ~(ALIGN - 1));
	}

private:
	struct obj {
		struct obj* next;
	};

private:
	static obj* free_list[NFREELISTS];
	static size_t FREELIST_INDEX(size_t bytes) {
		return (((bytes) + ALIGN - 1) / ALIGN - 1);
	}

	static void* refill(size_t n);
	static char* chunk_alloc(size_t size, int& nobjs);

	static char* start_free;     // 内存池的头
	static char* end_free;       // 内存池的尾

	static size_t heap_size;     // 分配累计量
public:
	static void* allocate(size_t n) {
		obj** my_free_list;
		obj* result;

		if(n > MAXBYTES) {
			return ::operator new(n);
		}

		my_free_list = free_list + FREELIST_INDEX(n);
		result = *my_free_list;
		if(result == 0) {
			void* r = refill(ROUND_UP(n));
			return r;
		}

		*my_free_list = result->next;
		return result;
	}

	static void deallocate(void* p, size_t n) {
		obj* q = (obj*)p;
		obj** my_free_list;

		if(n > MAXBYTES) {
			::operator delete(p);
			return;
		}

		my_free_list = free_list + FREELIST_INDEX(n);
		q->next = *my_free_list;
		*my_free_list = q;
	}
};

#endif