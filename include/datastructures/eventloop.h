#include <uv.h>

struct EventLoop {
  uv_loop_t *loop;
  uv_async_t recursive_directory_walk_handle;
  
  EventLoop() {
      this->loop = uv_default_loop();
      uv_async_init(loop, &recursive_directory_walk_handle, recursive_directory_walk_callback);
  }
};

