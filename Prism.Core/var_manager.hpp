#pragma once
#include <unordered_map>
#include <any>

class VarManager
{
private:
	std::unordered_map<std::string, std::any> var_storage;

public:
	void Clear();
	int Size();
	bool IsEmpty();
	bool IsExist(std::string key);
	bool Erase(std::string key);

public:
	bool get_var_as_bool(std::string key);
	void set_var_as_bool(std::string key, bool value);
	int get_var_as_int(std::string key);
	void set_var_as_int(std::string key, int value);
	__int64 get_var_as_int64(std::string key);
	void set_var_as_int64(std::string key, __int64 value);
	float get_var_as_float(std::string key);
	void set_var_as_float(std::string key, float value);
	double get_var_as_double(std::string key);
	void set_var_as_double(std::string key, double value);

};

extern VarManager* g_pVarManager;