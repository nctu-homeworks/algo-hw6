#include <fstream>
#include <msgpack.hpp>
#include <vector>
#include <algorithm>
using namespace std;

vector<long long int> targets_vector(100000, 0);
long long int targets[100000];
int workAry1[100000];
int workAry2[100000];

int main() {
	ifstream iFile("input.txt", ios::binary | ios::in);	

	iFile.seekg(0, ios::end);
	int length = iFile.tellg();
	iFile.seekg(0, ios::beg);
	
	msgpack::unpacker inPack;
	inPack.reserve_buffer(length);
	iFile.read(inPack.buffer(), inPack.buffer_capacity());
	inPack.buffer_consumed(length);
	
	iFile.close();

	msgpack::unpacked result;
	inPack.next(result);
	int problem_count;
	result.get() >> problem_count;

	ofstream oFile("output.txt", ios::out | ios::binary);
	msgpack::packer<ofstream> outPack(oFile);

	while(problem_count--) {
		inPack.next(result);
		result.get() >> targets_vector;
		int length = targets_vector.size();
		copy(targets_vector.begin(), targets_vector.end(), targets);
		long long int max_sum;
		int *index;

		/* targets = r15
		 * prevPrevMax_sum = r8
		 * prevMax_sum = r9
		 * prevPrevMax_index(start) = r10
		 * prevMax_index(start) = r11
		 * prevPrevMax_index = r12
		 * prevMax_index = r13
		 * should_copy = r14
		 */
		asm volatile(
			"    mov %%ebx, %%ecx;"
			/* Fetching targets */
			"    mov $targets, %%r15;"
			"    dec %%ebx;"
			"    shl $3, %%ebx;"
			"    add %%rbx, %%r15;"
			/* Initializing prevPrevMax */
			"    mov $0, %%r8;"
			"    mov $workAry1, %%r10;"
			"    mov %%r10, %%r12;"
			/* Initializing prevMax */
			"    mov 0(%%r15), %%r9;"
			"    mov $workAry2, %%r11;"
			"    mov %%r11, %%r13;"
			"    cmp $0, %%r9d;"
			"    jng C3;"
			"    mov %%ecx, 0(%%r13);"
			"    add $4, %%r13;"
			/* Get into the loop */
			"C3: mov $0, %%r14;"
			"    sub $8, %%r15;"
			"    dec %%ecx;"
			"    cmp $0, %%ecx;"
			"    je C4;"
			"L0: mov 0(%%r15), %%rax;"
			"    add %%r8, %%rax;" // targets[i] + prevPrevMax_sum
			"    cmp %%r9, %%rax;" // targets[i] + prevPrevMax_sum > prevMax_sum ?
			"    jg C0;"
			/* The target is discarded */
			"    mov %%r9, %%r8;"
			"    mov $1, %%r14;"
			"    jmp C1;"
			/* The target may shoot */
			"C0: cmp $1, %%r14d;" // Need to duplicate prevMax to prevPrevMax ?
			"    jne C2;"
			/* Duplicate prevMax_index to prevPrevMax_index */
			"    mov $0, %%r14;"
			"    mov %%rcx, %%rdx;"
			"    mov %%r13, %%rcx;"
			"    sub %%r11, %%rcx;"
			"    mov %%rcx, %%r12;"
			"    add %%r10, %%r12;"
			"    shr $2, %%rcx;"
			"    mov %%r10, %%rdi;"
			"    mov %%r11, %%rsi;"
			"    cld;"
			"    rep movsd;"
			"    mov %%rdx, %%rcx;"
			/* Accumulate the score and add to shoot list */
			"C2: mov 0(%%r15), %%rax;"
			"    add %%rax, %%r8;"
			"    mov %%ecx, 0(%%r12);"
			"    add $4, %%r12;"
			/* Swap prevMax <-> prevPrevMax */
			"    mov %%r8, %%rax;"
			"    mov %%r9, %%r8;"
			"    mov %%rax, %%r9;"
			"    mov %%r10, %%rax;"
			"    mov %%r11, %%r10;"
			"    mov %%rax, %%r11;"
			"    mov %%r12, %%rax;"
			"    mov %%r13, %%r12;"
			"    mov %%rax, %%r13;"
			/* Loop the loop */
			"C1: sub $8, %%r15;"
			"    loop L0;"
			/* Return the result */
			"C4: mov %%r9, %%rax;"
			"    mov %%r11, %%rdi;"
			"    mov %%r13, %%rbx;"
			"    sub %%r11, %%rbx;"
			"    shr $2, %%rbx;"

			: "=a"(max_sum), "=b"(length), "=D"(index)
			: "b"(length)
		);

		outPack << max_sum;
		outPack.pack_array(length);
		while(length--)
			outPack.pack(index[length]);
	}

	return 0;
}


