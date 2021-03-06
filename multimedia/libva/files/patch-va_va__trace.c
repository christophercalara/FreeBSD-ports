- Implement gettid() for BSDs

--- va/va_trace.c.orig	2019-07-05 13:14:31 UTC
+++ va/va_trace.c
@@ -48,12 +48,42 @@
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
-#include <sys/syscall.h>
 #include <pthread.h>
 #include <unistd.h>
 #include <sys/time.h>
 #include <errno.h>
 
+#if defined(__linux__)
+#include <sys/syscall.h>
+#elif defined(__DragonFly__) || defined(__FreeBSD__)
+#include <pthread_np.h>
+#elif defined(__NetBSD__)
+#include <lwp.h>
+#elif defined(__sun)
+#include <thread.h>
+#else // fallback
+#include <stdint.h>
+#endif
+
+#if !defined(__BIONIC__)
+static pid_t gettid()
+{
+#if defined(__linux__)
+  return syscall(__NR_gettid);
+#elif defined(__DragonFly__) || defined(__FreeBSD__)
+  return pthread_getthreadid_np();
+#elif defined(__NetBSD__)
+  return _lwp_self();
+#elif defined(__OpenBSD__)
+  return getthrid();
+#elif defined(__sun)
+  return thr_self();
+#else // fallback
+  return (intptr_t) pthread_self();
+#endif
+}
+#endif
+
 /*
  * Env. to debug some issue, e.g. the decode/encode issue in a video conference scenerio:
  * .LIBVA_TRACE=log_file: general VA parameters saved into log_file
@@ -290,7 +314,7 @@ static void add_trace_config_info(
 {
     struct trace_config_info *pconfig_info;
     int idx = 0;
-    pid_t thd_id = syscall(__NR_gettid);
+    pid_t thd_id = gettid();
 
     LOCK_RESOURCE(pva_trace);
 
@@ -668,7 +692,7 @@ static struct trace_log_file *start_tracing2log_file(
 {
     struct trace_log_files_manager *plog_files_mgr = NULL;
     struct trace_log_file *plog_file = NULL;
-    pid_t thd_id = syscall(__NR_gettid);
+    pid_t thd_id = gettid();
     int i = 0;
 
     LOCK_RESOURCE(pva_trace);
@@ -707,7 +731,7 @@ static void refresh_log_file(
     struct trace_context *ptra_ctx)
 {
     struct trace_log_file *plog_file = NULL;
-    pid_t thd_id = syscall(__NR_gettid);
+    pid_t thd_id = gettid();
     int i = 0;
 
     plog_file = ptra_ctx->plog_file;
@@ -1238,7 +1262,7 @@ static void internal_TraceUpdateContext (
 {
     struct trace_context *trace_ctx = NULL;
     int i = 0, delete = 1;
-    pid_t thd_id = syscall(__NR_gettid);
+    pid_t thd_id = gettid();
 
     if(tra_ctx_idx >= MAX_TRACE_CTX_NUM)
         return;
