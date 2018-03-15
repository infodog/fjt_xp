##
##  Makefile -- Build procedure for sample fjt Apache module
##  Autogenerated via ``apxs -n fjt -g''.
##


#下面的参数需要根据实际的路径进行调整
builddir=.
top_srcdir=/Users/zhengxiangyang/work/fjt/thirdParty/dist/httpd-2.4.29
top_builddir=/Users/zhengxiangyang/work/fjt/thirdParty/dist/httpd-2.4.29
include /Users/zhengxiangyang/work/fjt/thirdParty/dist/httpd-2.4.29/build/special.mk

#   the used tools
APACHECTL=apachectl

#   additional defines, includes and libraries
#DEFS=-Dmy_define=my_value
#INCLUDES=-Imy/include/dir
#LIBS=-Lmy/lib/dir -lmylib

#   the default target
all: local-shared-build

#   install the shared object file into Apache 
install: install-modules-yes

#   cleanup
clean:
	-rm -f mod_fjt.o mod_fjt.lo mod_fjt.slo mod_fjt.la 

#   simple test
test: reload
	lynx -mime_header http://localhost/fjt

#   install and activate shared object by reloading Apache to
#   force a reload of the shared object file
reload: install restart

#   the general Apache start/restart/stop
#   procedures
start:
	$(APACHECTL) start
restart:
	$(APACHECTL) restart
stop:
	$(APACHECTL) stop

