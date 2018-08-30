#include <cstdio>
#include <cstdint>
#include <Windows.h>

#include "zlib/zlib.h"


typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

constexpr uint32 c_pigg_file_sig = 0x123;
constexpr uint32 c_internal_file_sig = 0x3456;
constexpr uint32 c_string_table_sig = 0x6789;
constexpr uint32 c_slot_table_sig = 0x9abc;
constexpr uint32 c_invalid_id = (uint32)-1;

constexpr uint32 kilobytes(uint32 n) { return n * 1024; }
constexpr uint32 megabytes(uint32 n) { return kilobytes(n) * 1024; }

#define assert(x) if(!(x)) {int* p = 0; *p = 0;}


struct Pigg_Entry
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
};


uint32 copy_string(char* dst, const char* src)
{
	char* dst_iter = dst;
	while (*src)
	{
		*dst_iter = *src;
		++src;
		++dst_iter;
	}
	*dst_iter = 0;

	return (uint32)(dst_iter - dst);
}


int main()
{
	const char* file_name = "geomV1.pigg";

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

	Pigg_Entry* entries = new Pigg_Entry[num_entries];

	for (uint32 i = 0; i < num_entries; ++i)
	{
		Pigg_Entry* entry = &entries[i];

		success = ReadFile(file, &entry->sig, sizeof(entry->sig), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		assert(entry->sig == c_internal_file_sig);
		success = ReadFile(file, &entry->string_id, sizeof(entry->string_id), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &entry->file_size, sizeof(entry->file_size), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &entry->timestamp, sizeof(entry->timestamp), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &entry->offset, sizeof(entry->offset), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &entry->unknown, sizeof(entry->unknown), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &entry->slot_id, sizeof(entry->slot_id), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, entry->md5, 16, &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		success = ReadFile(file, &entry->compressed_size, sizeof(entry->compressed_size), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
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

	char** strings = new char*[num_strings];
	uint32 string_data_size = table_size - (num_strings * 4);
	char* string_data = new char[string_data_size];
	uint32 num_table_bytes_read = 0;

	for (uint32 i = 0; i < num_strings; ++i)
	{
		uint32 string_length;
		success = ReadFile(file, &string_length, sizeof(string_length), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		assert(string_length <= (string_data_size - num_bytes_read));

		success = ReadFile(file, &string_data[num_table_bytes_read], string_length, &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);

		strings[i] = &string_data[num_table_bytes_read];

		num_table_bytes_read += string_length;
	}

	// not sure what this does
	uint32 slot_table_sig;
	success = ReadFile(file, &slot_table_sig, sizeof(slot_table_sig), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);
	assert(slot_table_sig == c_slot_table_sig);
	uint32 num_slots;
	success = ReadFile(file, &num_slots, sizeof(num_slots), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);
	success = ReadFile(file, &table_size, sizeof(table_size), &num_bytes_read, /*lpOverlapped*/ nullptr);
	assert(success);

	uint8** packed_meta = new uint8*[num_slots];
	uint32* packed_meta_size = new uint32[num_slots]; // todo(jbr) lots of this needs renaming and compressing
	uint32 total_packed_meta_size = table_size - (num_slots * 4);
	uint8* packed_meta_data = new uint8[total_packed_meta_size];
	num_table_bytes_read = 0;

	for (uint32 i = 0; i < num_slots; ++i)
	{
		success = ReadFile(file, &packed_meta_size[i], sizeof(packed_meta_size[i]), &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);
		assert(packed_meta_size[i] <= (total_packed_meta_size - num_bytes_read));

		success = ReadFile(file, &packed_meta_data[num_table_bytes_read], packed_meta_size[i], &num_bytes_read, /*lpOverlapped*/ nullptr);
		assert(success);

		packed_meta[i] = &packed_meta_data[num_table_bytes_read];

		num_table_bytes_read += packed_meta_size[i];
	}
	
	constexpr uint32 c_in_buffer_size = megabytes(1);
	constexpr uint32 c_out_buffer_size = megabytes(32);

	uint8* in_buffer = new uint8[c_in_buffer_size];
	uint8* out_buffer = new uint8[c_out_buffer_size];
	for (uint32 i = 0; i < num_entries; ++i)
	{
		Pigg_Entry* entry = &entries[i];

		assert(entry->compressed_size <= c_in_buffer_size);
		assert(entry->file_size <= c_out_buffer_size);

		SetFilePointer(file, entry->offset, /*lpDistanceToMoveHigh*/ nullptr, FILE_BEGIN);

		if (entry->compressed_size == 0)
		{
			success = ReadFile(file, out_buffer, entry->file_size, &num_bytes_read, /*lpOverlapped*/ nullptr);
			assert(success);
		}
		else
		{
			assert(entry->compressed_size != entry->file_size); // I *think* uncompressed data will have compressed_size of 0, but asserting in case I'm wrong

			success = ReadFile(file, in_buffer, entry->compressed_size, &num_bytes_read, /*lpOverlapped*/ nullptr);
			assert(success);

			z_stream zlib_stream;
			zlib_stream.zalloc = Z_NULL;
			zlib_stream.zfree = Z_NULL;
			zlib_stream.opaque = Z_NULL;
			zlib_stream.avail_in = 0;
			zlib_stream.next_in = Z_NULL;
			int zlib_result = inflateInit(&zlib_stream);
			assert(zlib_result == Z_OK);

			zlib_stream.next_in = in_buffer;
			zlib_stream.avail_in = entry->compressed_size;
			zlib_stream.next_out = out_buffer;
			zlib_stream.avail_out = entry->file_size;
			zlib_result = inflate(&zlib_stream, Z_NO_FLUSH);
			assert(zlib_result == Z_STREAM_END);
			(void)inflateEnd(&zlib_stream);
		}

		assert(entry->string_id < num_strings);

		char* file_name = strings[entry->string_id];

		for (uint32 string_i = 0; file_name[string_i]; ++string_i)
		{
			if (file_name[string_i] == '/')
			{
				// temporarily terminate string here to create directory, then reinstate it after
				file_name[string_i] = 0;

				success = CreateDirectoryA(file_name, /*lpSecurityAttributes*/ nullptr);
				assert(success || GetLastError() == ERROR_ALREADY_EXISTS);

				file_name[string_i] = '/';
			}
		}
		
		HANDLE out_file = CreateFileA(strings[entry->string_id], GENERIC_WRITE, FILE_SHARE_WRITE, /*lpSecurityAttributes*/ nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, /*hTemplateFile*/ nullptr);
		DWORD num_bytes_written;
		success = WriteFile(out_file, out_buffer, entry->file_size, &num_bytes_written, /*lpOverlapped*/ nullptr);
		assert(success);
		CloseHandle(out_file);

		if (entry->slot_id != c_invalid_id)
		{
			char buffer[512];
			uint32 chars_copied = copy_string(buffer, file_name);
			copy_string(&buffer[chars_copied], ".meta");
			out_file = CreateFileA(buffer, GENERIC_WRITE, FILE_SHARE_WRITE, /*lpSecurityAttributes*/ nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, /*hTemplateFile*/ nullptr);

			uint8* meta = packed_meta[entry->slot_id];
			uint32 meta_size = packed_meta_size[entry->slot_id];

			uint32* meta_file_size = (uint32*)meta;
			if (*meta_file_size == meta_size)
			{
				// not compressed
				success = WriteFile(out_file, &meta[4], meta_size - 4, &num_bytes_written, /*lpOverlapped*/ nullptr);
				assert(success);
			}
			else
			{
				assert((*meta_file_size + 4) == meta_size);
				uint32* compressed_size = (uint32*)&meta[4];

				if (!(*compressed_size))
				{
					// not compressed
					success = WriteFile(out_file, &meta[8], meta_size - 8, &num_bytes_written, /*lpOverlapped*/ nullptr);
					assert(success);
				}
				else
				{
					z_stream zlib_stream;
					zlib_stream.zalloc = Z_NULL;
					zlib_stream.zfree = Z_NULL;
					zlib_stream.opaque = Z_NULL;
					zlib_stream.avail_in = 0;
					zlib_stream.next_in = Z_NULL;
					int zlib_result = inflateInit(&zlib_stream);
					assert(zlib_result == Z_OK);

					zlib_stream.next_in = &meta[8];
					zlib_stream.avail_in = meta_size - 8;
					zlib_stream.next_out = out_buffer;
					zlib_stream.avail_out = c_out_buffer_size;
					zlib_result = inflate(&zlib_stream, Z_NO_FLUSH);
					assert(zlib_result == Z_STREAM_END);

					success = WriteFile(out_file, out_buffer, c_out_buffer_size - zlib_stream.avail_out, &num_bytes_written, /*lpOverlapped*/ nullptr);
					assert(success);

					(void)inflateEnd(&zlib_stream);
				}
			}

			CloseHandle(out_file);
		}
	}

    return 0;
}

