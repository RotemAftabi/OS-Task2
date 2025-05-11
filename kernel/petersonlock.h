#ifndef _PETERSONLOCK_H_
#define _PETERSONLOCK_H_

struct petersonlock {
  int flag[2];
  int turn;
  int in_use;
};

#endif // _PETERSONLOCK_H_
