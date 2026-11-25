#ifdef _WIN32
#define DL_TEST_API __declspec(dllexport)
#else
#define DL_TEST_API __attribute__((visibility("default")))
#endif

extern "C" DL_TEST_API int dl_test_entry_point() { return 42; }

extern "C" DL_TEST_API int dl_test_other_symbol() { return 7; }
