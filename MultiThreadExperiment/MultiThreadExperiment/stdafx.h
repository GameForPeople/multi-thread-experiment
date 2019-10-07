#pragma once

//Essential
#include <iostream>
#include <chrono>

 // for CRITICAL_SECTION
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
//
//// C++ 11
#include <atomic>
#include <thread>
#include <mutex>
#include <future>
//
//// C++ 17
#include <shared_mutex>
//
//// PPL
#include <concurrent_queue.h>
#include <concurrent_priority_queue.h>
#include <concurrent_vector.h>
#include <concurrent_unordered_map.h>
#include <concurrent_unordered_set.h>
//
#include <unordered_set>
//
//// ������ ����������, ĸ�� �̹��� ����� ����..
using namespace std;
using namespace std::chrono;
using namespace Concurrency;