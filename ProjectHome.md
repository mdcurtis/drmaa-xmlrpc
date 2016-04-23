drmaa-xmlrpc is a quick XMLRPC wrapper around the DRMAA API implemented by most popular cluster schedulers. It is written in C and depends on xmlrpc-c and your scheduler's DRMAA implementation library. The end daemon is an abyss webserver serving out standard XMLRPC calls. API follows DRMAAc bindings - same function names and same arguments (omit any buffers meant to return values).

I have made the decision to never implement the blocking functions - synchronize() and wait() - because of various problems it would raise with TCP connection keep alives. Even if it wasn't a problem in any given particular case, it would invite bad code that wouldn't scale. So if you need blocking functionality just poll. Whenever DRMAAv2 comes out and I decide to rewrite this stuff, I will revisit whether I will implement blocking API.

Look at the wiki for build and use instructions.

UPDATE 6/5/2011: In case anybody is wondering if this project is dead since the lack of updates - I simply have had no reason to change anything. If you're having issues please post it in the issues tab.

&lt;wiki:gadget url="http://www.ohloh.net/p/486245/widgets/project\_users.xml?style=rainbow" height="100" border="0"/&gt;