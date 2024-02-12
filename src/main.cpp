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
#include "lib.h"

namespace chr = std::chrono;
int main(int argc, char **argv){
  // make stdin non blocking.
  fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

  enableRawMode();
  initEditor();

  disableRawMode();

  if (argc >= 2) { g_editor.original_cwd = fs::path(argv[1]); }

  if (argc >= 2) {
    g_editor.original_cwd = fs::canonical(fs::path(argv[1]));
  } else {
    g_editor.original_cwd = fs::absolute(fs::current_path());
  }

  if (fs::is_regular_file(g_editor.original_cwd)) {
    const fs::path filepath = g_editor.original_cwd;
    g_editor.original_cwd = g_editor.original_cwd.remove_filename();
    g_editor.original_cwd = ctrlpGetGoodRootDirAbsolute(g_editor.original_cwd);
    g_editor.getOrOpenNewFile(FileLocation(filepath, Cursor(0, 0)));
    g_editor.getOrOpenNewFile(FileLocation(filepath, Cursor(0, 0)));
  } else {
    g_editor.original_cwd = ctrlpGetGoodRootDirAbsolute(g_editor.original_cwd);
    ctrlpOpen(&g_editor.ctrlp, VM_NORMAL, g_editor.original_cwd);
  }

  tilde::tildeWrite("original_cwd: '%s'", g_editor.original_cwd.c_str());

  enableRawMode();
  while (1) {
    auto tbegin = Debouncer::get_time();

    editorDraw();
    editorProcessKeypress();
    editorTickPostKeypress();

    auto tend = Debouncer::get_time();
    chr::nanoseconds elapsed_nsec = tend - tbegin;
    auto elapsed_sec = chr::duration_cast<chr::seconds>(elapsed_nsec);
    if (elapsed_sec.count() > 0) { continue; }

    auto elapsed_microsec = chr::duration_cast<chr::microseconds>(elapsed_nsec);
    const chr::microseconds total_microsec(1000000 / 120); // 120 FPS = 1s / 120
    // frames = 1000000 microsec / 120 frames
    std::this_thread::sleep_for(total_microsec - elapsed_microsec);

  };
  disableRawMode();
  return 0;
}
