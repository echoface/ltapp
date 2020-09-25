#include <iostream>
#include "async_mysql/mysql_async_con.h"
#include "async_mysql/mysql_client_impl.h"
#include "gflags/gflags.h"
#include "gflags/gflags_declare.h"
#include "glog/logging.h"

using namespace lt;

MysqlOptions option = {
  //.host = "10.9.158.21",
  .host = "127.0.0.1",
  .port = 3306,
  .user = "root",
  .passwd = "",
  .dbname = "mysql",
  .query_timeout = 5000,
};

DEFINE_string(host, "host", "spec a mysql host");

int main(int argc, char** argv) {
  mysql_library_init(0, NULL, NULL);

  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  FLAGS_stderrthreshold=google::INFO;
  FLAGS_colorlogtostderr=true;

  base::MessageLoop loop;
  loop.SetLoopName("main");
  loop.Start();

  if (!FLAGS_host.empty()) {
    option.host = FLAGS_host;
  }
  auto client = std::shared_ptr<MysqlAsyncClientImpl>(new MysqlAsyncClientImpl(&loop, 1));
  client->InitWithOption(option);

  auto callback = [&](RefQuerySession qs)->void {
    std::ostringstream oss;
    for (const auto&  field : qs->ColumnHeaders()) {
      oss <<  field << " \t| ";
    }
    for (const auto& row : qs->Result()) {
      oss << "\n";
      for (auto& v : row) {
        oss << v << "\t| ";
      }
    }
    LOG(INFO) << "query:" << qs->QueryContent()
      << "\ncode:" << qs->Code()
      << "\nmessage:" << qs->ErrorMessage()
      << "\nresultcount:" << qs->ResultRows()
      << "\naffectedline:" << qs->AffectedRows()
      << "\nresult:\n" << oss.str();
  };


  std::string content;
  while(content != "exit") {
    std::getline(std::cin, content);
    LOG(INFO) << "got query content:" << content;

    if (content != "exit") {
      RefQuerySession qs = QuerySession::New();
      qs->UseDB("mysql").Query(content).Then(std::bind(callback, qs));
      client->PendingQuery(qs);
    } else {
      client->Close();
      client.reset();

      loop.QuitLoop();
    }
  }

  loop.WaitLoopEnd();

  mysql_library_end();
  return 0;
}

