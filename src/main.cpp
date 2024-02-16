#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <iterator>
#include <thread>
#include "datastructures/editorconfig.h"
#include "datastructures/debouncer.h"
#include "datastructures/recursivedirectorywalker.h"
#include "lib.h"
#include <set>

#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

namespace fs = std::filesystem;

struct Watcher {
  struct watcher_fs_event_t {
      uv_fs_event_t fs_event;
      Watcher *watcher;
  };
  
  uv_loop_t *loop;
  RecursiveDirectoryWalker directoryWalker;
  Watcher(uv_loop_t *loop, fs::path path) :
	  loop(loop), directoryWalker(path) {};
  std::set<const char *> edited_paths;

  void tick() {
    std::optional<fs::path> p = directoryWalker.tick();
    if (!p) { return; }
    std::cerr << "watching '" << *p << "'\n";

    Watcher::watcher_fs_event_t *req = NULL;
	    // (Watcher::watcher_fs_event_t *) malloc(sizeof(Watcher::watcher_fs_event_t));
    req->watcher = this;
    uv_fs_event_init(loop, (uv_fs_event_t*)req);
    uv_fs_event_start((uv_fs_event_t*)req, Watcher::on_change, p->string().c_str(),
		    /*FS_EVENT_FLAGS*/UV_FS_EVENT_RECURSIVE);
  }


  static void on_change(uv_fs_event_t *_handle, const char *filename, int events, int status) {
    char path[1024];
    size_t size = 1023;
    uv_fs_event_getpath(_handle, path, &size);
    Watcher::watcher_fs_event_t *handle = (Watcher::watcher_fs_event_t *)_handle;

    path[size] = '\0';

    if (!filename) { return; }
    // TODO: check if file is deleted and give up.
    // TODO: handle new file being created in directory
    // TODO: handle new directory being created to register an event for the new directory.
    printf("Change detected in %s/%s\n", path, filename);
    handle->watcher->edited_paths.insert(filename);
  }
};

Watcher *g_watcher;

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("expected usage: elid <path-of-directory-to-monitor>");
    exit(1);
  }
  uv_loop_t *loop = uv_default_loop();

  const char *dir_to_watch = argv[1];
  g_watcher = new Watcher(loop, dir_to_watch);

  while(true) {
    g_watcher->tick();
    uv_run(loop, UV_RUN_NOWAIT);
  }
  return 0;
}

// namespace chr = std::chrono;
// int main(int argc, char **argv){
//   // make stdin non blocking.
//   fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
//
//   enableRawMode();
//   initEditor();
//
//   disableRawMode();
//
//   if (argc >= 2) { g_editor.original_cwd = fs::path(argv[1]); }
//
//   if (argc >= 2) {
//     g_editor.original_cwd = fs::canonical(fs::path(argv[1]));
//   } else {
//     g_editor.original_cwd = fs::absolute(fs::current_path());
//   }
//
//   if (fs::is_regular_file(g_editor.original_cwd)) {
//     const fs::path filepath = g_editor.original_cwd;
//     g_editor.original_cwd = g_editor.original_cwd.remove_filename();
//     g_editor.original_cwd = ctrlpGetGoodRootDirAbsolute(g_editor.original_cwd);
//     g_editor.getOrOpenNewFile(FileLocation(filepath, Cursor(0, 0)));
//     g_editor.getOrOpenNewFile(FileLocation(filepath, Cursor(0, 0)));
//   } else {
//     g_editor.original_cwd = ctrlpGetGoodRootDirAbsolute(g_editor.original_cwd);
//     ctrlpOpen(&g_editor.ctrlp, VM_NORMAL, g_editor.original_cwd);
//   }
//
//   tilde::tildeWrite("original_cwd: '%s'", g_editor.original_cwd.c_str());
//
//   enableRawMode();
//   while (1) {
//     auto tbegin = Debouncer::get_time();
//
//     editorDraw();
//     editorProcessKeypress();
//     editorTickPostKeypress();
//
//     auto tend = Debouncer::get_time();
//     chr::nanoseconds elapsed_nsec = tend - tbegin;
//     auto elapsed_sec = chr::duration_cast<chr::seconds>(elapsed_nsec);
//     if (elapsed_sec.count() > 0) { continue; }
//
//     auto elapsed_microsec = chr::duration_cast<chr::microseconds>(elapsed_nsec);
//     const chr::microseconds total_microsec(1000000 / 120); // 120 FPS = 1s / 120
//     // frames = 1000000 microsec / 120 frames
//     std::this_thread::sleep_for(total_microsec - elapsed_microsec);
//
//   };
//   disableRawMode();
//   return 0;
// }
