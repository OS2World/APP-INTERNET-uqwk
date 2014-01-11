rem !CRF! - create tcpip library for emx/gcc
cd \tcpip\lib
emximp -o so32dll.imp so32dll.lib
emximp -o tcp32dll.imp tcp32dll.lib
emximp -o rpc32dll.imp rpc32dll.lib
emximp -o dpi32dll.imp dpi32dll.lib
cat so32dll.imp tcp32dll.imp rpc32dll.imp dpi32dll.imp | grep -v -i ioctl > tcpip.imp
emximp -o tcpip.a tcpip.imp
emximp -o tcpip.lib tcpip.imp

