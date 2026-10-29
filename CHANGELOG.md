# Changelog

This file records the changes made to the starter code and explain why the issues happened and how we fixed them. Your own ideas and explanations should write in the report, or demonstrate in the short video.

## - 2026-10-05

Initial publish of the starter code for COMP3421/COMP9415 26T3 Computer Graphics group assignment to the staff repos from individual assignment repo.

## - 2026-10-29

In `CMakeLists.txt`, switched stb to a source archive of the same pinned commit to avoid CMake's incompatibility between commit hashes and `GIT_SHALLOW` and reduce download size by omitting Git history. The version remains pinned to prevent unexpected upstream updates from introducing compatibility issues. Thanks to @Weiren ZHONG, @Shengkui Hu and other students for reporting this issue.
