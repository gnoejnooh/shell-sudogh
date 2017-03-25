# Shell and Chat (C Programming)
## 320sh: Mini-shell with basic operations (a small subset of Bash's functionality)
#### Launch shell with following command `./320sh`
#### To disable command key capture and job control on the parent shell, use `. ./launcher.sh`
+ The built-in functionalities include: `cd`,`pwd`,`echo`,`set`,`help`
+ The shell keeps previously typed commands, and allow a user to easily re-issue them with up or down arrow key.
+ Backspace and arrow key is supported.
+ Command line argument with quote is supported. (e.g. `320sh> ./a.out "This is one argument"` )
+ Output redirection is supported. It can be accomplshed by three special characters `<`,`>`, and `|`.
```
Redirection Example
ls -l | sort -k2n
ls -l | grep 2 | wc -l
./a.out < b.txt > out.txt
```
