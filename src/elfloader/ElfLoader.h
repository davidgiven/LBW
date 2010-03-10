/* © 2010 David Given.
 * LBW is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#ifndef ELFLOADER_H
#define ELFLOADER_H

#include "elf.h"

class FD;

class ElfLoader
{
public:
	ElfLoader();
	~ElfLoader();

	void Open(const string& filename);
	void Load();
	void Close();

public:
	const struct elfhdr& GetElfHeader() const { return _elfhdr; }
	const struct elf_phdr& GetProgramHeader(int n) const { return _phdr[n]; }
	size_t GetNumProgramHeaders() const { return _elfhdr.e_phnum; }
	size_t GetProgramHeaderSize() const { return _elfhdr.e_phentsize; }
	void* GetEntrypoint() const { return (void*) _entrypoint; }
	bool HasInterpreter() const { return !_interpreter.empty(); }
	const string& GetInterpreter() const { return _interpreter; }

private:
	Ref<FD> _fd;
	struct elfhdr _elfhdr;
	struct elf_phdr* _phdr;
	string _interpreter;
	u32 _entrypoint;
};

#endif
