#include "var_manager.hpp"

VarManager* g_pVarManager = new VarManager();

void VarManager::Clear()
{
	var_storage.clear();
}

int VarManager::Size()
{
	return var_storage.size();
}

bool VarManager::IsEmpty()
{
	return var_storage.empty();
}

bool VarManager::IsExist(std::string key)
{
	return var_storage.find(key) != var_storage.end();
}

bool VarManager::Erase(std::string key)
{
	return var_storage.erase(key);
}

bool VarManager::get_var_as_bool(std::string key)
{
	if (var_storage[key].type() == typeid(bool))
		return std::any_cast<bool>(var_storage[key]);
	return NULL;
}

void VarManager::set_var_as_bool(std::string key, bool value)
{
	var_storage[key] = value;
}

int VarManager::get_var_as_int(std::string key)
{
	if (var_storage[key].type() == typeid(int))
		return std::any_cast<int>(var_storage[key]);
	return NULL;
}

void VarManager::set_var_as_int(std::string key, int value)
{
	var_storage[key] = value;
}

__int64 VarManager::get_var_as_int64(std::string key)
{
	if (var_storage[key].type() == typeid(__int64))
		return std::any_cast<__int64>(var_storage[key]);
	return NULL;
}

void VarManager::set_var_as_int64(std::string key, __int64 value)
{
	var_storage[key] = value;
}

float VarManager::get_var_as_float(std::string key)
{
	if (var_storage[key].type() == typeid(float))
		return std::any_cast<float>(var_storage[key]);
	return NULL;
}

void VarManager::set_var_as_float(std::string key, float value)
{
	var_storage[key] = value;
}

double VarManager::get_var_as_double(std::string key)
{
	if (var_storage[key].type() == typeid(double))
		return std::any_cast<double>(var_storage[key]);
	return NULL;
}

void VarManager::set_var_as_double(std::string key, double value)
{
	var_storage[key] = value;
}