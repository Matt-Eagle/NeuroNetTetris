#include "stdafx.h"

#include "RandomHelper.h"
#include <chrono>

using namespace std;
//static initialization
default_random_engine RandomHelper::generator = default_random_engine(static_cast<uint>(chrono::steady_clock::now().time_since_epoch().count()));