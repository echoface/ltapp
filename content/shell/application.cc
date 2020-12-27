#include "application.h"
#include <chrono>
#include <functional>
#include <gflags/gflags.h>
#include <base/coroutine/wait_group.h>
#include <base/message_loop/message_loop.h>
#include <base/coroutine/coroutine_runner.h>

#include <memory>
#include <mutex>
#include <iostream>
#include <vector>

namespace lt {

BootTaskPtr BootTask::New(const string& name,
                         BoolTask boot_task,
                         ExcRunType run_type) {
  BootTaskPtr task(new BootTask);
  task->name = name;
  task->task = boot_task;
  task->exc_type = run_type;
  return std::move(task);
}

Application::Application(AppContext* context)
  :context_(context){
  main_.SetLoopName(context->Name());
}

Application::~Application() {}

Application& Application::Prepare(BootTaskPtr item) {
  on_start_.push_back(std::move(item));
  return *this;
}

Application& Application::Cleanup(BootTaskPtr item) {
  on_finish_.push_back(std::move(item));
  return *this;
}

void Application::Run() {
  main_.Start();

  bool success = false;
  CO_GO &main_ << [&, this]() {
    success = RunBootupTask();
  };
  std::cout << " start...", std::cout.flush();
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (cv_.wait_for(lock,std::chrono::milliseconds(500)) == std::cv_status::timeout) {
      std::cout << '.', std::cout.flush();
    };
  }

  if (!success) {
    std::cout << " preapare task failed, exit..." << std::endl;
    return;
  }

  std::cout << " application start up!" << std::endl;

  context_->OnStart();

  main_.WaitLoopEnd();
  LOG(INFO) << " APPLICATION LOOP END";

  context_->OnStop();

  success = false;
  CO_GO &main_ << [&]() {
    RunCleanupTask();
  };
  std::cout << " stop...", std::cout.flush();
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (std::cv_status::timeout ==
           cv_.wait_for(lock, std::chrono::microseconds(100))) {
      std::cout << '.';
      std::cout.flush();
    }
  }
  std::cout << " application finish done!" << std::endl;
}

bool Application::RunCleanupTask() {

  std::shared_ptr<base::WaitGroup> wg = base::WaitGroup::New();

  std::vector<BootTask*> failed_tasks_;

  for (auto& item : on_finish_) {
    LOG(INFO) << "run cleanup task:" << item->name;
    bool success = false;
    switch(item->exc_type) {
      case ExcRunType::kSequenceType:
        if (!item->task()) {
          failed_tasks_.push_back(item.get());
        }
        break;
      case ExcRunType::kAsyncRunType:
      case ExcRunType::kAsyncCoroType: {

        BootTask* _task = item.get();
        auto async_run_fn = [&, wg, _task]() {
          if (!_task->task()) {
            failed_tasks_.push_back(_task);
          }
          wg->Done();
        };

        wg->Add(1);
        if (item->exc_type == ExcRunType::kAsyncCoroType) {
          CO_GO &main_ << async_run_fn;
        } else {
          main_.PostTask(NewClosure(async_run_fn));
        }
      }break;
      default:
        break;
    }
  }

  wg->Wait();
  for (auto task : failed_tasks_) {
    LOG(ERROR) << __func__ << " Cleanup Task:" << task->name << " Failed";
  }
  cv_.notify_all();
  return failed_tasks_.empty();
}

bool Application::RunBootupTask() {

  std::shared_ptr<base::WaitGroup> wg = base::WaitGroup::New();

  std::vector<BootTask*> failed_tasks_;

  for (auto& item : on_start_) {
    LOG(INFO) << "run prepare task:" << item->name;
    bool success = false;
    switch(item->exc_type) {
      case ExcRunType::kSequenceType: {
        if (!item->task()) {
          failed_tasks_.push_back(item.get());
        }
      }break;
      case ExcRunType::kAsyncRunType:
      case ExcRunType::kAsyncCoroType: {
        BootTask* _task = item.get();

        auto async_run_fn = [&, _task, wg]() {
          if (!_task->task()) {
            failed_tasks_.push_back(_task);
          }
          wg->Done();
        };
        wg->Add(1);
        if (item->exc_type == ExcRunType::kAsyncCoroType) {
          CO_GO &main_ << async_run_fn;
        } else {
          main_.PostTask(NewClosure(async_run_fn));
        }
      }break;
      default:
        break;
    }
  }

  wg->Wait();
  for (auto task : failed_tasks_) {
    LOG(ERROR) << "PrepareTask:" << task->name << " Failed";
  }
  cv_.notify_all();
  return failed_tasks_.empty();
}

} // namespace lt
