#include <cstdio>
#include <cstdint>
#include <Windows.h>


typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

constexpr uint32 c_pigg_file_sig = 0x123;
constexpr uint32 c_internal_file_sig = 0x3456;
constexpr uint32 c_string_table_sig = 0x6789;
constexpr uint32 c_slot_table_sig = 0x9abc;

#define assert(x) if(!(x)) {int* p = 0; *p = 0;}


int main()
{
	const char* file_name = "misc.pigg";

	OFSTRUCT file_info = {};
	HANDLE file = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, /*lpSecurityAttributes*/ nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, /*hTemplateFile*/ nullptr);

	uint32 file_sig;
	uint16 unknown;
	uint16 version;
	uint16 header_size;
	uint16 used_header_bytes;
	uint32 num_entries;

	DWORD num_bytes_read;
	bool success = ReadFile(file, &file_sig, sizeof(file_sig), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);
	assert(file_sig == c_pigg_file_sig);
	success = ReadFile(file, &unknown, sizeof(unknown), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);
	success = ReadFile(file, &version, sizeof(version), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);
	success = ReadFile(file, &header_size, sizeof(header_size), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);
	assert(header_size == 16);
	success = ReadFile(file, &used_header_bytes, sizeof(used_header_bytes), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);
	assert(used_header_bytes == 48);
	success = ReadFile(file, &num_entries, sizeof(num_entries), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);

	for (uint32 i = 0; i < num_entries; ++i)
	{
		uint32 sig;
		uint32 string_id;
		uint32 file_size;
		uint32 timestamp;
		uint32 offset;
		uint32 unknown;
		uint32 slot_id;
		uint8 md5[16];
		uint32 compressed_size;

		success = ReadFile(file, &sig, sizeof(sig), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		assert(sig == c_internal_file_sig);
		success = ReadFile(file, &string_id, sizeof(string_id), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &file_size, sizeof(file_size), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &timestamp, sizeof(timestamp), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &offset, sizeof(offset), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &unknown, sizeof(unknown), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &slot_id, sizeof(slot_id), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, md5, 16, &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &compressed_size, sizeof(compressed_size), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);

		// if compressed_size == 0 then it's not compressed
	}

	uint32 string_table_sig;
	success = ReadFile(file, &string_table_sig, sizeof(string_table_sig), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);
	assert(string_table_sig == c_string_table_sig);
	uint32 num_strings;
	success = ReadFile(file, &num_strings, sizeof(num_strings), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);
	uint32 table_size;
	success = ReadFile(file, &table_size, sizeof(table_size), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);

	char buffer[512];
	for (uint32 i = 0; i < num_strings; ++i)
	{
		uint32 string_length;
		success = ReadFile(file, &string_length, sizeof(string_length), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		assert(string_length <= sizeof(buffer));
		success = ReadFile(file, buffer, string_length, &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
	}

	// not sure what this does
	uint32 slot_table_sig;
	success = ReadFile(file, &slot_table_sig, sizeof(slot_table_sig), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);
	assert(slot_table_sig == c_slot_table_sig);
	uint32 num_slots;
	success = ReadFile(file, &num_slots, sizeof(num_slots), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);
	assert(num_slots == 0); // lets see when we find a pigg with some of these
	success = ReadFile(file, &table_size, sizeof(table_size), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);

	char* data = new char[1024 * 1024];
	for (uint32 i = 0; i < num_slots; ++i)
	{
		uint32 data_length;
		success = ReadFile(file, &data_length, sizeof(data_length), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		assert(data_length <= (1024 * 1024));
		success = ReadFile(file, buffer, data_length, &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
	}


    return 0;
}

