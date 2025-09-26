#ifndef lacpp_sorted_list_hpp
#define lacpp_sorted_list_hpp lacpp_sorted_list_hpp
#include <mutex>

/* a sorted list implementation by David Klaftenegger, 2015
 * please report bugs or suggest improvements to david.klaftenegger@it.uu.se
 */

class TATAS{
	std::atomic<bool> locked{false};
	public:
		void lock()
		{
			bool occupied = false;
			do{
				while(locked.load(std::memory_order_relaxed)){}
				occupied = locked.exchange(true, std::memory_order_acquire);
			}while(occupied);

		}
		void unlock()
		{
			locked.store(false, std::memory_order_release);
		}
};

class Qnode{
	public:
		std::atomic<bool> locked{false};
};

class CLHLock{
	std::atomic<Qnode*> tail;
	inline static thread_local Qnode* myNode = new Qnode{};
	inline static thread_local Qnode* pred = nullptr;
	public:
		void lock(){
			myNode->locked.store(true, std::memory_order_relaxed);
			pred = tail.exchange(myNode, std::memory_order_acq_rel);
			while (pred->locked.load(std::memory_order_acquire)) {}

		}
		void unlock(){
			myNode->locked.store(false, std::memory_order_release);
			myNode = pred;
		}
};

/* struct for list nodes */
template<typename T>
struct node {
	T value;
	node<T>* next;
	CLHLock m;
};


/* non-concurrent sorted singly-linked list */
template<typename T>
class sorted_list {
	node<T>* first = nullptr;
	CLHLock first_m;

	public:
		/* default implementations:
		 * default constructor
		 * copy constructor (note: shallow copy)
		 * move constructor
		 * copy assignment operator (note: shallow copy)
		 * move assignment operator
		 *
		 * The first is required due to the others,
		 * which are explicitly listed due to the rule of five.
		 */
		sorted_list() = default;
		sorted_list(const sorted_list<T>& other) = default;
		sorted_list(sorted_list<T>&& other) = default;
		sorted_list<T>& operator=(const sorted_list<T>& other) = default;
		sorted_list<T>& operator=(sorted_list<T>&& other) = default;
		~sorted_list() {
			while(first != nullptr) {
				remove(first->value);
			}
		}
		/* insert v into the list */
		void insert(T v) {
			/* first find position */
			node<T>* pred = nullptr;
			node<T>* garbage = nullptr;
			bool has_first = false;
			first_m.lock();
			has_first = true;
			if(first) first->m.lock();
			node<T>* succ = first;
			while(succ != nullptr && succ->value < v) {
				garbage = pred;
				pred = succ;
				if(succ->next) succ->next->m.lock();
				succ = succ->next;
				if(garbage) garbage->m.unlock();
				if (has_first && pred != nullptr) {
            		first_m.unlock();
            		has_first = false;
        		}
			}
			
			/* construct new node */
			node<T>* current = new node<T>();
			current->value = v;

			/* insert new node between pred and succ */
			current->next = succ;
			if(pred == nullptr) {
				first = current;
				if (has_first) { first_m.unlock(); has_first = false; }
			} else {
				pred->next = current;
			}
			if(succ) succ->m.unlock();
			if(pred) pred->m.unlock();
			if(has_first) first_m.unlock();
		}

		void remove(T v) {
			/* first find position */
			node<T>* pred = nullptr;
			node<T>* next = nullptr;
			node<T>* garbage = nullptr;
			bool has_first = false;
			first_m.lock();
			has_first = true;
			if(first)
			{
				first->m.lock();
				if(first->next) first->next->m.lock();
				next = first -> next;
			}
			node<T>* current = first;

			while(current != nullptr && current->value < v) {
				garbage = pred;
				pred = current;
				current = next;
				if(current) 
				{
					if(current->next)current->next->m.lock();
					next = current->next;
				}else next = nullptr;

				if(garbage) garbage->m.unlock();

				if (has_first && pred != nullptr) {
            		first_m.unlock();
					has_first = false;
        		}
			}
			if(current == nullptr || current->value != v) {
				/* v not found */
				if(next) next->m.unlock();
				if(current) current->m.unlock();
				if(pred) pred->m.unlock();
				if(has_first) first_m.unlock();
				return;
			}
			/* remove current */
			if(pred == nullptr) {
				first = current->next;
				if (has_first) { first_m.unlock(); has_first = false; }
			} else {
				pred->next = current->next;
			}
			if(next) next->m.unlock();
			current->m.unlock();
			if(pred) pred->m.unlock();
			delete current;
			if(has_first)first_m.unlock();
		}

		/* count elements with value v in the list */
		std::size_t count(T v) {
			std::size_t cnt = 0;
			bool has_first = false;
			/* first go to value v */
			first_m.lock();
			has_first = true;
			if(first){
				first->m.lock();
				first_m.unlock();
				has_first = false;
			}
			node<T>* current = first;
			node<T>* garbage = nullptr;
			while(current != nullptr && current->value < v) {
				garbage = current;
				if(current->next) current->next->m.lock();
				current = current->next;
				garbage->m.unlock();
			}
			/* count elements */
			while(current != nullptr && current->value == v) {
				cnt++;
				garbage = current;
				if(current->next) current->next->m.lock();
				current = current->next;
				garbage->m.unlock();
			}
			if(current) current->m.unlock();
			if(has_first) first_m.unlock();
			return cnt;
		}
};

#endif // lacpp_sorted_list_hpp
