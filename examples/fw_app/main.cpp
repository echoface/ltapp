#include "stdio.h"
#include "fw_context.h"

int main(int argc, char** argv) {
  lt::FWContext context;  
  lt::LtApplication app(&context);

  return 0;
}
