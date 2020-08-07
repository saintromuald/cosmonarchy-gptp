@echo off
git update-index --skip-worktree post_build_hyd_debug.bat
git update-index --skip-worktree post_build_hyd_release.bat
git update-index --skip-worktree post_build_dlv_debug.bat
git update-index --skip-worktree post_build_dlv_release.bat