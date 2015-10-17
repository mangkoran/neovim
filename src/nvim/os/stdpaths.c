#include <stdbool.h>

#include "nvim/os/os.h"
#include "nvim/path.h"
#include "nvim/memory.h"

typedef enum {
  kXDGConfigHome,
  kXDGDataHome,
  kXDGCacheHome,
  kXDGRuntimeDir,
  kXDGConfigDirs,
  kXDGDataDirs,
} XDGDirType;

static const char *xdg_env_vars[] = {
  [kXDGConfigHome] = "XDG_CONFIG_HOME",
  [kXDGDataHome] = "XDG_DATA_HOME",
  [kXDGCacheHome] = "XDG_CACHE_HOME",
  [kXDGRuntimeDir] = "XDG_RUNTIME_DIR",
  [kXDGConfigDirs] = "XDG_CONFIG_DIRS",
  [kXDGDataDirs] = "XDG_DATA_DIRS",
};

static const char *const xdg_defaults[] = {
  // Windows, Apple stuff are just shims right now
#ifdef WIN32
  // Windows
#elif APPLE
  // Apple (this includes iOS, which we might need to handle differently)
  [kXDGConfigHome] = "~/Library/Preferences",
  [kXDGDataHome] = "~/Library/Application Support",
  [kXDGCacheHome] = "~/Library/Caches",
  [kXDGRuntimeDir] = "~/Library/Application Support",
  [kXDGConfigDirs] = "/Library/Application Support",
  [kXDGDataDirs] = "/Library/Application Support",
#else
  // Linux, BSD, CYGWIN
  [kXDGConfigHome] = "~/.config",
  [kXDGDataHome] = "~/.local/share",
  [kXDGCacheHome] = "~/.cache",
  [kXDGRuntimeDir] = "",
  [kXDGConfigDirs] = "/etc/xdg/",
  [kXDGDataDirs] = "/usr/local/share/:/usr/share/",
};
#endif

static char *get_xdg(const XDGDirType idx)
  FUNC_ATTR_WARN_UNUSED_RESULT
{
  const char *const env = xdg_env_vars[idx];
  const char *const fallback = xdg_defaults[idx];

  const char *const env_val = os_getenv(env);
  char *ret = NULL;
  if (env_val != NULL) {
    ret = xstrdup(env_val);
  } else if (fallback) {
    ret = (char *) expand_env_save((char_u *)fallback);
  }

  return ret;
}

static char *get_xdg_home(XDGDirType idx)
  FUNC_ATTR_WARN_UNUSED_RESULT
{
  char *dir = get_xdg(idx);
  if (dir) {
    dir = concat_fnames(dir, "nvim", true);
  }
  return dir;
}

static void create_dir(const char *dir, int mode, const char *suffix)
  FUNC_ATTR_NONNULL_ALL
{
  char *failed;
  if (!os_mkdir_recurse(dir, mode, &failed)) {
    // TODO: Create a folder in $TMPDIR instead
    DLOG("Create dir failed");
  }
}

char *get_user_conf_dir(void)
  FUNC_ATTR_WARN_UNUSED_RESULT
{
  return get_xdg_home(kXDGConfigHome);
}

char *get_from_user_conf(const char *fname)
  FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
  return concat_fnames(get_user_conf_dir(), fname, true);
}

char *get_user_data_dir(void)
  FUNC_ATTR_WARN_UNUSED_RESULT
{
  return get_xdg_home(kXDGDataHome);
}

char *get_from_user_data(const char *fname)
  FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
  char *dir = concat_fnames(get_user_data_dir(), fname, true);
  if (!os_isdir((char_u *)dir)) {
    create_dir(dir, 0755, fname);
  }
  return dir;
}
