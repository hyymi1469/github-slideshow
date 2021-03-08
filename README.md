server(C++)
======
__Lock-Free IOCP Server(Windows)__
- When deal with receive packet after operating GetQueuedCompletionStatus Function, you don't have to use lock(like mutex, critical section ...).

__Packet of Stream Type for Send/receive__
-  you can use variable size type like std::string when packet send, receive, because not using the struct. 

__MultiThread pool manage__
- utilize std::map for thread manage, you can check current thread state

※ Required install the boost_1_75_0 Ver
