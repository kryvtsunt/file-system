File System

In this assignment you will build a FUSE filesystem driver that will let you mount a 1MB disk image (data file) as a filesystem.

Step 0: Get a dev environment
The FUSE library and tools are not installed on the CCIS Linux server, so for this assignment you should work on a local Linux system. Your submission will be tested on an Ubuntu 16.04 system (which has FUSE version 2.6).

If you cannot get a local Ubuntu VM working at all, email the instructor to request an account on the cs3650 test server.

Step 1: Implement a basic filesystem
You should extend the provided starter code so that it lets you do the following:

Create files.
List the files in the filesystem root directory (where you mounted it).
Write to small files (under 4k).
Read from small files (under 4k).
Delete files.
Rename files.
This will require that you come up with a structure for how the file system will store data in it's 1MB "disk". See the filesystem slides - especially the original ext filesystem - for inspiration.

Some additional code that might be useful is provided in the "hints" directory in the assignment download. You can use this if you want. Using a block structure and mmap to access the filesystem image file as shown in the "pages.c" file is recommended.

Step 2: Add missing functionality
Extend your filesystem to do more stuff:

Create hard links - multiple names for the same file blocks.
Read and write from files larger than one block. For example, you should be able to support fifty 1k files or five 100k files.
Create directories and nested directories. Directory depth should only be limited by space.
Remove directories.
Support metadata (permissions and timestamps) for files and directories.
Step 3: Report
Write a report.txt. Include at least the following:
What advantages and drawbacks does your filesystem have? How would you improve it if you had more time?
What features did you complete? What is still missing? Did you implement any additional functionality that wasn't required?
