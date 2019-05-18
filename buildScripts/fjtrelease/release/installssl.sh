if test ! -e /lib64/libssl.so.1.1 ; then
    cp ./fjtv3ssl/openssl-1.1.1a/lib/libssl.so.1.1 /lib64/
fi

if test ! -e /lib64/libcrypto.so.1.1 ; then
    cp ./fjtv3ssl/openssl-1.1.1a/lib/libcrypto.so.1.1 /lib64/
fi