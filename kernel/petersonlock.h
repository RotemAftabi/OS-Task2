struct petersonlock {
  int flag[2];
  int turn;
  int in_use;
};

void init_peterson_locks(void);
int peterson_create(void);
int peterson_acquire(int lock_id, int role);
int peterson_release(int lock_id, int role);
int peterson_destroy(int lock_id);

