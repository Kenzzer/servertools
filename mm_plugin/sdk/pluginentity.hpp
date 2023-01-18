#pragma once

#include "baseentity.hpp"

template<typename T, unsigned int N>
class CMetaModEntity : public T
{
private:
	char _padding[N];
};