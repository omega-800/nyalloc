#include "nyalloc.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void test_zero_size(void) {
  assert(nyalloc(0) == NULL);
  printf("ok: nyalloc(0) == NULL\n");
}

static void test_basic_alloc(void) {
  char *p = nyalloc(16);
  assert(p != NULL);
  strcpy(p, "hello world");
  assert(strcmp(p, "hello world") == 0);
  fwee(p);
  printf("ok: basic alloc/free\n");
}

static void test_many_alloc(void) {
  enum { N = 1000 };
  void *ptrs[N];
  for (int i = 0; i < N; ++i) {
    size_t sz = (size_t)(i * 31 % 997) + 1;
    ptrs[i] = nyalloc(sz);
    assert(ptrs[i] != NULL);
    memset(ptrs[i], (int)(i % 256), sz);
  }
  for (int i = 0; i < N; ++i) {
    size_t sz = (size_t)(i * 31 % 997) + 1;
    unsigned char *p = ptrs[i];
    for (size_t j = 0; j < sz; ++j)
      assert(p[j] == (unsigned char)(i % 256));
  }
  for (int i = 0; i < N; ++i)
    fwee(ptrs[i]);
  printf("ok: many allocations with contents intact\n");
}

static void test_fwee_null(void) {
  fwee(NULL);
  printf("ok: fwee(NULL) is a no-op\n");
}

static void test_reuse(void) {
  char *a = nyalloc(64);
  assert(a != NULL);
  memset(a, 'A', 64);
  fwee(a);

  char *b = nyalloc(64);
  assert(b != NULL);
  memset(b, 'B', 64);
  fwee(b);
  printf("ok: freed block can be reused\n");
}

static void test_interleaved(void) {
  enum { N = 100 };
  void *ptrs[N] = { 0 };
  for (int i = 0; i < N; ++i) {
    ptrs[i] = nyalloc(1);
    assert(ptrs[i] != NULL);
    if (i % 2 == 0)
      fwee(ptrs[i]);
  }
  for (int i = 0; i < N; ++i)
    fwee(ptrs[i]);
  printf("ok: interleaved alloc/free\n");
}

static void test_large_alloc(void) {
  size_t sz = 1 << 20;
  unsigned char *p = nyalloc(sz);
  assert(p != NULL);
  memset(p, 0xAB, sz);
  for (size_t i = 0; i < sz; i += 4096)
    assert(p[i] == 0xAB);
  fwee(p);
  printf("ok: large allocation\n");
}

static void test_alignment(void) {
  enum { N = 500 };
  for (int i = 0; i < N; ++i) {
    void *p = nyalloc(1);
    assert(p != NULL);
    assert(((uintptr_t)p % sizeof(void *)) == 0);
    fwee(p);
  }
  printf("ok: pointers aligned to sizeof(void *)\n");
}

int main(void) {
  test_zero_size();
  test_basic_alloc();
  test_many_alloc();
  test_fwee_null();
  test_reuse();
  test_interleaved();
  test_large_alloc();
  test_alignment();
  printf("all tests passed\n");
  return 0;
}