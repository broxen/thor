#include <cstdio>
#include <cstdint>
#include <Windows.h>


typedef uint32_t uint32;

constexpr uint32 c_pigg_file_sig = 0x123;

#define assert(x) if(!(x)) {int* p = 0; *p = 0;}


int main()
{
	const char* file_name = "texts.pigg";

	OFSTRUCT file_info = {};
	HANDLE file = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, /*lpSecurityAttributes*/ nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, /*hTemplateFile*/ nullptr);

	uint32 file_sig;
	DWORD num_bytes_read;
	bool success = ReadFile(file, &file_sig, sizeof(file_sig), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);

	// skip 8
	SetFilePointer(file, 8, /*lpDistanceToMoveHigh*/ nullptr, FILE_CURRENT);

	uint32 num_directory_entries;
	success = ReadFile(file, &num_directory_entries, sizeof(num_directory_entries), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);

	for (int i = 0; i < num_directory_entries; ++i)
	{
		// 7 uint32
		// 16 chars
		// 1 uint32
		uint32 u0;
		uint32 string_id;
		uint32 file_size;
		uint32 timestamp;
		uint32 offset;
		uint32 u5;
		uint32 slot_id;
		char md5[17];
		md5[16] = 0;
		uint32 compressed_size;

		success = ReadFile(file, &u0, sizeof(u0), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &string_id, sizeof(string_id), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &file_size, sizeof(file_size), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &timestamp, sizeof(timestamp), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &offset, sizeof(offset), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &u5, sizeof(u5), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &slot_id, sizeof(slot_id), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, md5, 16, &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &compressed_size, sizeof(compressed_size), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
	}

    return 0;
}

