#pragma once

#include <unordered_map>
#include <string>

class LibInfo
{
public:
private:
	std::unordered_map<std::string, void*> _symbols;
};

class ScannerModule
{
public:
	ScannerModule(void* module);

	void* FindSignature(const std::string& signature);
	void* FindSymbol(const std::string& name);

private:
	void* _module;
	std::unordered_map<std::string, void*> _symbols;
};