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
//// 안좋은 습관이지만, 캡쳐 이미지 사이즈를 위해..
using namespace std;
using namespace std::chrono;
using namespace Concurrency;