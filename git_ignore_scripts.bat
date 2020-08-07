@echo off
git update-index --skip-worktree post_build_debug_hyd.bat
git update-index --skip-worktree post_build_release_hyd.bat
git update-index --skip-worktree post_build_debug_dlv.bat
git update-index --skip-worktree post_build_release_dlv.bat