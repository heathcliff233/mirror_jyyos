f(int x, int y) {
  int z = x * y;
  if (z == 18) {
    bug();
  }
}

int main() {
  int x = nondet();
  int y = x * 2;
  f(x, y);
}
