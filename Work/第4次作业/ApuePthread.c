/*
 * Copyright 2015 Weihongkai
 */


#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <string.h>
#include <pthread.h>


typedef void (*special_task_function)(void *);
struct Queue *qp_result;
pthread_mutex_t kQlock1 = PTHREAD_MUTEX_INITIALIZER;

typedef struct data {
  int num1;
  int num2;
  char cal[10];
  double result;
  struct data *next;
} special_data;

typedef struct Queue {
  special_data *head;
  special_data *tail;
} queue;

typedef struct thread_pool_conf {
  unsigned int tasks_max;
  unsigned int threads_current;
  unsigned int threads_stack_size;
} special_thread_conf;

typedef struct thread_task {
  void *argv;
  special_task_function handler;
  struct pthread_task *next_task;
} special_task;

typedef struct thread_queue {
  special_task *first;
  special_task **last;

  unsigned int tasks_max;
  unsigned int tasks_current;
} special_queue_task;

typedef struct thread_pool {
  pthread_mutex_t mtx;
  pthread_cond_t cond;
  special_queue_task queue_tasks;

  unsigned threads_current;
  unsigned threads_stack_size;
} special_thread_pool;

int special_thread_pool_create(special_thread_pool *pool);
void special_thread_pool_destroy(special_thread_pool *pool);

int special_thread_pool_conf_para_check(special_thread_conf *conf);
special_thread_pool *special_thread_pool_init(special_thread_conf *conf);

void special_queue_task_init(special_queue_task *queue_task);
int special_thread_pool_task_append(special_thread_pool *pool, special_task_function handler, void *argv);
int special_thread_pool_tasks_append_by_judge(special_thread_pool *pool, void *argv);

int special_thread_add_plus(special_thread_pool *pool);
int special_thread_add(special_thread_pool *pool);

int special_thread_mutex_create(pthread_mutex_t *mutex);

static void *special_thread_pool_cycle(void *argv);
void special_thread_pool_cleanup_tf(void *argv);
void cal_plus(void *data);
void cal_sub(void *data);
void cal_multi(void *data);
void cal_div(void *data);

void result_queue_append(queue *qp, special_data *data_append);
void print_result(queue *qp);


int
special_thread_pool_create(special_thread_pool *pool) {
  int i = 0;
  int flag;
  pthread_t pid;
  pthread_attr_t attr;

  if (pthread_attr_init(&attr) != 0) {
    return -1;
  }

  if (pool->threads_stack_size != 0) {
    if (pthread_attr_setstacksize(&attr, pool->threads_stack_size) != 0) {
      pthread_attr_destroy(&attr);
      return -1;
    }
  }

  for (; i < pool->threads_current; ++i) {
    flag = pthread_create(&pid, &attr, special_thread_pool_cycle, pool);
    if (flag != 0) {
      printf("can't create thread: %s\n", strerror(flag));
      return -1;
    }
  }
  pthread_attr_destroy(&attr);
  return 0;
}

void
special_thread_pool_destroy(special_thread_pool *pool) {
  unsigned int n;
  volatile unsigned int lock;
  special_task task_temp;

  task_temp.handler = special_thread_pool_cleanup_tf;
  task_temp.argv = (void *) &lock;
  for (n = 0; n < pool->threads_current; n++) {
    lock = 1;
    if (special_thread_pool_task_append(pool, task_temp.handler, task_temp.argv) != 0) {
      return;
    }
    while (lock) {
      usleep(1);
    }
  }
  pthread_cond_destroy(&pool->cond);
  pthread_mutex_destroy(&pool->mtx);
  free(pool);
}

int
special_thread_pool_conf_para_check(special_thread_conf *conf) {
  if (conf == NULL) {
    return -1;
  }

  if (conf->threads_current < 1) {
    return -1;
  }

  if (conf->tasks_max < 1) {
    conf->tasks_max = 999999;
  }
  return 0;
}

special_thread_pool *
special_thread_pool_init(special_thread_conf *conf) {
  int flag;
  special_thread_pool *pool = NULL;
  pthread_key_t key;
  pthread_attr_t attr;

  if ((flag = special_thread_pool_conf_para_check(conf)) == -1) {
    return NULL;
  }

  if ((pool = (special_thread_pool *) malloc(sizeof(special_thread_pool))) == NULL) {
    return NULL;
  }

  //Init the thread pool para
  pool->threads_stack_size = conf->threads_stack_size;
  pool->threads_current = conf->threads_current;
  pool->queue_tasks.tasks_max = conf->tasks_max;
  pool->queue_tasks.tasks_current = 0;

  special_queue_task_init(&pool->queue_tasks);

  if ((flag = special_thread_mutex_create(&pool->mtx)) != 0) {
    free(pool);
    return NULL;
  }

  if ((flag = pthread_key_create(&key, NULL)) != 0) {
    pthread_mutex_destroy(&pool->mtx);
    free(pool);
    return NULL;
  }

  if ((flag = pthread_cond_init(&pool->cond, NULL)) != 0) {
    pthread_mutex_destroy(&pool->mtx);
    pthread_key_delete(key);
    free(pool);
    return NULL;
  }

  if ((flag = special_thread_pool_create(pool)) != 0) {
    pthread_mutex_destroy(&pool->mtx);
    pthread_key_delete(key);
    pthread_cond_destroy(&pool->cond);
    free(pool);
    return NULL;
  }
  return pool;
}

void
special_queue_task_init(special_queue_task *queue_task) {
  queue_task->first = NULL;
  queue_task->last = &queue_task->first;
}

int
special_thread_pool_task_append(special_thread_pool *pool, special_task_function handler, void *argv) {
  special_task *task = NULL;

  if ((task = (special_task *) malloc(sizeof(special_task))) == NULL) {
    return -1;
  }

  task->handler = handler;
  task->argv = argv;
  task->next_task = NULL;

  if (pthread_mutex_lock(&pool->mtx) != 0) { //加锁
    free(task);
    return -1;
  }
  do {

    if ((pool->queue_tasks.tasks_current) >= (pool->queue_tasks.tasks_max)) {
      break;
    }

    *pool->queue_tasks.last = task;
    pool->queue_tasks.last = &task->next_task;
    pool->queue_tasks.tasks_current++;

    if (pthread_cond_signal(&pool->cond) != 0) {
      break;
    }
    pthread_mutex_unlock(&pool->mtx);
    return 0;

  } while (0);
  pthread_mutex_unlock(&pool->mtx);
  free(task);
  return -1;

}

int
special_thread_pool_tasks_append_by_judge(special_thread_pool *pool, void *argv) {
  int flag = 0;
  special_data *data_temp = (special_data *) argv;
  if (strcmp(data_temp->cal, "+") == 0) {

    flag = special_thread_pool_task_append(pool, cal_plus, argv);

  } else if (strcmp(data_temp->cal, "-") == 0) {

    flag = special_thread_pool_task_append(pool, cal_sub, argv);

  } else if (strcmp(data_temp->cal, "*") == 0) {

    flag = special_thread_pool_task_append(pool, cal_multi, argv);

  } else {

    if (data_temp->num2 == 0) {

      printf("%d / %d, Number2 Can Not Be Zero\n", data_temp->num1, data_temp->num2);

      data_temp->result = -1;

    } else {

      flag = special_thread_pool_task_append(pool, cal_div, argv);

    }

  }
  return flag;
}

int
special_thread_add_plus(special_thread_pool *pool) {
  pthread_t pid;
  pthread_attr_t attr;
  int ret = 0;
  if (pthread_attr_init(&attr) != 0) {
    return -1;
  }
  if (pool->threads_stack_size != 0) {
    if (pthread_attr_setstacksize(&attr, pool->threads_stack_size) != 0) {
      pthread_attr_destroy(&attr);
      return -1;
    }
  }
  ret = pthread_create(&pid, &attr, special_thread_pool_cycle, pool);
  if (ret == 0) {
    pool->threads_current++;
  }
  pthread_attr_destroy(&attr);
  return ret;
}

int
special_thread_add(special_thread_pool *pool) {
  int ret = 0;
  if (pthread_mutex_lock(&pool->mtx) != 0) {
    return -1;
  }
  ret = special_thread_add_plus(pool);
  pthread_mutex_unlock(&pool->mtx);
  return ret;
}

int special_thread_mutex_create(pthread_mutex_t *mutex) {
  int ret = 0;
  pthread_mutexattr_t attr;

  if (pthread_mutexattr_init(&attr) != 0) {
    return -1;
  }

  if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) != 0) {
    pthread_mutexattr_destroy(&attr);
    return -1;
  }

  ret = pthread_mutex_init(mutex, &attr);

  pthread_mutexattr_destroy(&attr);

  return ret;
}

static void *
special_thread_pool_cycle(void *argv) {
  unsigned int exit_flag = 0;
  sigset_t set;
  special_task *ptask = NULL;
  special_thread_pool *pool = argv;

  sigfillset(&set);
  sigdelset(&set, SIGILL);
  sigdelset(&set, SIGFPE);
  sigdelset(&set, SIGSEGV);
  sigdelset(&set, SIGBUS);

  if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0) {
    return (void *) 0;
  }
  while (!exit_flag) {
    if (pthread_mutex_lock(&pool->mtx) != 0) {
      return (void *) 0;
    }

    while (pool->queue_tasks.first == NULL) {
      if (pthread_cond_wait(&pool->cond, &pool->mtx) != 0) {
        pthread_mutex_unlock(&pool->mtx);
        return (void *) 0;
      }
    }

    ptask = pool->queue_tasks.first;
    pool->queue_tasks.first = ptask->next_task;
    pool->queue_tasks.tasks_current--;

    if (pool->queue_tasks.first == NULL) {
      pool->queue_tasks.last = &pool->queue_tasks.first;
    }

    if (pthread_mutex_unlock(&pool->mtx) != 0) {
      return (void *) 0;
    }

    ptask->handler(ptask->argv);
    free(ptask);
    ptask = NULL;
  }
}

void
special_thread_pool_cleanup_tf(void *argv) {
  unsigned int *lock = argv;
  *lock = 0;
  pthread_exit(0);
}

void
cal_plus(void *data) {
  special_data *tmp = (special_data *) data;
  tmp->result = tmp->num1 + tmp->num2;
  pthread_mutex_lock(&kQlock1);
  result_queue_append(qp_result, data);
  pthread_mutex_unlock(&kQlock1);
}

void
cal_sub(void *data) {
  special_data *tmp = (special_data *) data;
  tmp->result = tmp->num1 - tmp->num2;
  pthread_mutex_lock(&kQlock1);
  result_queue_append(qp_result, data);
  pthread_mutex_unlock(&kQlock1);
}

void
cal_multi(void *data) {
  special_data *tmp = (special_data *) data;
  tmp->result = tmp->num1 * tmp->num2;
  pthread_mutex_lock(&kQlock1);
  result_queue_append(qp_result, data);
  pthread_mutex_unlock(&kQlock1);
}

void
cal_div(void *data) {
  special_data *tmp = (special_data *) data;
  tmp->result = (double)tmp->num1 / (double)tmp->num2;
  pthread_mutex_lock(&kQlock1);
  result_queue_append(qp_result, data);
  pthread_mutex_unlock(&kQlock1);
}

void
result_queue_append(queue *qp, special_data *data_append) {
  data_append->next = NULL;

  if (qp->tail != NULL) {

    qp->tail->next = data_append;

  } else {

    qp->head = data_append;

  }

  qp->tail = data_append;
}


void
print_result(queue *qp) {

  special_data *data_temp;

  printf("Answers are:\n");

  for (data_temp = qp->head; data_temp != NULL; data_temp = data_temp->next) {

    printf("%d %s %d = %lf\n", data_temp->num1, data_temp->cal, data_temp->num2, data_temp->result);

  }

}

int
main(int argc, char *argv[]) {
  int fd;
  int num1, num2;
  int i = 0;
  char str[10];

  special_data *data_temp;

  if (argc != 2) {

    printf("input error\n");

    exit(0);

  }
  if ((fd = open(argv[1], O_RDONLY)) < 0) {

    printf("open the %s file error\n", argv[1]);

    exit(0);

  }

  dup2(fd, STDIN_FILENO);
  //init the result queue
  qp_result = (queue *) malloc(sizeof(queue));
  qp_result->head = NULL;
  qp_result->tail = NULL;

  //init the thread pool
  special_thread_conf conf = {5, 5, 8192};
  special_thread_pool *pool = special_thread_pool_init(&conf);
  if (pool == NULL) {
    return 0;
  }

  while (scanf("%d%s%d", &num1, str, &num2) != EOF) {
    if ((data_temp = (special_data *) malloc(sizeof(special_data))) == NULL) {
      printf("pthread malloc occur error!!!");
      exit(0);
    }
    data_temp->num1 = num1;
    data_temp->num2 = num2;
    strcpy(data_temp->cal, str);
    if ((special_thread_pool_tasks_append_by_judge(pool, data_temp)) != 0)
      return 0;
    i++;
    if (i == 50) {
      special_thread_add(pool);
      special_thread_add(pool);
    }
    if (i == 100) {
      special_thread_add(pool);
      special_thread_add(pool);
    }
  }
  pthread_mutex_destroy(&kQlock1);
  close(fd);
  special_thread_pool_destroy(pool);
  print_result(qp_result);
  return 0;
}