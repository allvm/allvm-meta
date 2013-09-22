
#include <vector>

extern "C" int foo(int a, int b, int c) {
  std::vector<int> ints;
  ints.push_back(a);
  ints.push_back(b);
  ints.push_back(c);

  int sum = 0;
  for (int i = 0; i < ints.size(); ++i)
    sum += ints[i];

  return sum;
}
