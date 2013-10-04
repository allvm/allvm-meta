
int abs(int i);
int abs(int i) {
  // XXX: abs(-INT_MIN) = -INT_MIN, per manpage.
  return i > 0 ? i : -i;
}


