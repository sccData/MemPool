#include "MemoryPool.h"

char* MemoryPool::start_free = 0;

char* MemoryPool::end_free = 0;

size_t MemoryPool::heap_size = 0;

obj* MemoryPool::free_list[NFREELISTS] 
	= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void* MemoryPool::refill(size_t n) {
	int nobjs = NNODES;
	char* chunk = chunk_alloc(n, nobjs);
	obj** my_free_list;
	obj* result;
	obj* current_obj;
	obj* next_obj;
	int i;

	if(1 == nobjs)
		return chunk;

	my_free_list = free_list + FREELIST_INDEX(n);
	result = (obj*)chunk;
	*my_free_list = next_obj = (obj*)(chunk + n);
	for(i=1; ; ++i) {
		current_obj = next_obj;
		next_obj = (obj*)((char*)next_obj + n);
		if(nobjs - 1 == i) {
			current_obj->next = 0;
			break;
		} else {
			current_obj->next = next_obj;
		}
	}

	return result;
}

char* MemoryPool::chunk_alloc(size_t size, int& nobjs) {
	char* result;
	size_t total_bytes = size * nobjs;
	size_t bytes_left = end_free - start_free;

	if(bytes_left >= total_bytes) {
		result = start_free;
		start_free += total_bytes;
		return result;
	} else if(bytes_left >= size) {
		nobjs = bytes_left / size;
		total_bytes = size * nobjs;
		result = start_free;
		start_free += total_bytes;
		return result;
	} else {
		size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
		if(bytes_left > 0) {
			obj** my_free_list = free_list + FREELIST_INDEX(bytes_left);
			((obj*)start_free)->next = *my_free_list;
			*my_free_list = (obj*)start_free;
		}

		start_free = (char*)malloc(bytes_to_get);
		if(0 == start_free) {
			int i;
			obj** my_free_list, *p;
			for(i = size + ALIGN; i < MAXBYTES; i += ALIGN) {
				my_free_list = free_list + FREELIST_INDEX(i);
				p = *my_free_list;
				if(0 != p) {
					*my_free_list = p->next;
					start_free = (char*)p;
					end_free = start_free + i;
					return chunk_alloc(size, nobjs);
				}
			}

			end_free = 0;
			start_free = (char*)::operator new(bytes_to_get);
		}

		heap_size += bytes_to_get;
		end_free = start_free + bytes_to_get;
		return chunk_alloc(size, nobjs);
	}
}