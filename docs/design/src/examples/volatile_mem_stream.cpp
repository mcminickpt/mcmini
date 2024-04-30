

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "mcmini/misc/volatile_mem_streambuf.hpp"
#include "mcmini/real_world/remote_address.hpp"
#include "mcmini/real_world/shm.hpp"

int main() {
  real_world::shared_memory_region smr{"hello", 100};

  volatile int *smr_bytes = smr.as_array_of<int>();

  // std::memset((void *)smr.get(), 0x22, smr.size());q
  volatile_mem_streambuf vms{smr.byte_array(20), 30};

  uint32_t my_val = UINT32_MAX;

  auto *j = std::cout.rdbuf(&vms);
  std::cout.write((char *)(&my_val), sizeof(my_val));
  std::cout.write((char *)(&my_val), sizeof(my_val));
  std::cout.flush();
  std::cout.rdbuf(j);

  j = std::cin.rdbuf(&vms);

  std::cin.read((char *)(&my_val), sizeof(my_val));

  std::cin.rdbuf(j);

  std::cout << "Value?: " << std::hex << my_val << " whoaa" << std::endl;

  system("xxd /dev/shm/hello");

  return 0;
}
