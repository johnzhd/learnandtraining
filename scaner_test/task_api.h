#pragma once

#include <signal.h>

#include <deque>
#include <string>
#include <vector>
#include <atomic>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition.hpp>


#include "task_data.h"
#include "task_work.h"
#include "policy_work.h"

