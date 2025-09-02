# CString-Like
Remake of the CString in Clearcode tm.

2 MB exe simply by using CString. NO way. 
So, I have rewritten the whole thing in what I call ClearCode. 
It's a strick style of writing code and method.
I have invented it in the 90's

I went from 2MB to 16K. Still big for a 32bit but acceptable.

# About UTF/UNICODE Support:
This version do not support nor care about UTF16/UTF32.
(It's kind of a mess right now. So, I guess I'm gonna have to fix that too.)
I might do a future release of a version that directly support UTF8. 
That's the only one that matters and care about. 

# About ties to windows.h 
I skipped all that part except for the mutex. 
Member function such as FormatMessage, LoadString.
I don't need, neither should you. 

# Note for linux Programmer: ... 
  You should use the Linux compatible version: DCLinuxString.h
  I haven't tested it please make sure you run the test. 
  *If it's not too much trouble let me know how it went. 
  
# How to use:
  Just add the DCString.h/DCLinuxString.h to your project. 
  






