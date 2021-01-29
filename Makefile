all:
	git add signals/signals.c signals/Makefile signals/signallib.h semaphores/reader.c semaphores/Makefile semaphores/writer.c semaphores/semaphoreslib.h pipe/reader.c pipe/writer.c pipe/Makefile pipe/fifolib.h children/msg_children.c  proxy/proxy.cpp proxy/proxylib.h proxy/Makefile
	git commit
	git push --set-upstream lunev master