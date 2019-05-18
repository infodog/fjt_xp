#yum install gcc-c++
#yum install expat-devel
#yum install automake
#yum install libtool

export home=$(pwd)
mkdir buildtemp

#dist=/home/fjtv3ssl
export dist=$home/buildtemp/fjtv3ssl
mkdir $dist

export httpd=httpd-2.4.38
export ssllib=openssl-1.1.1b
export ziplib=zlib-1.2.11
export pcrelib=pcre-8.43

export buildpath=$home/buildtemp/$httpd
export sslpath=$dist/$ssllib
export zlibpath=$dist/$ziplib
export pcrepath=$dist/$pcrelib

cd buildtemp
#get httpd 
httpdurl=http://mirrors.tuna.tsinghua.edu.cn/apache//httpd/httpd-2.4.38.tar.gz
curl $httpdurl --output httpd.tar.gz

#get apr http://mirror.bit.edu.cn/apache//apr/apr-1.6.5.tar.gz
aprurl=http://mirror.bit.edu.cn/apache//apr/apr-1.6.5.tar.gz
curl $aprurl --output apr.tar.gz


#get apr-util http://mirror.bit.edu.cn/apache//apr/apr-util-1.6.1.tar.gz
aprutilurl=http://mirror.bit.edu.cn/apache//apr/apr-util-1.6.1.tar.gz
curl $aprutilurl --output apr-util.tar.gz

#get open-ssl https://www.openssl.org/source/openssl-1.1.1b.tar.gz
opensslurl=https://www.openssl.org/source/openssl-1.1.1b.tar.gz
curl $opensslurl --output openssl.tar.gz

#get zlib https://www.zlib.net/zlib-1.2.11.tar.gz
zliburl=https://www.zlib.net/zlib-1.2.11.tar.gz
curl $zliburl --output zlib.tar.gz

#get pcre https://ftp.pcre.org/pub/pcre/pcre-8.43.zip
pcreurl=https://ftp.pcre.org/pub/pcre/pcre-8.43.zip
curl $pcreurl --output pcre.zip

mkdir httpd 
tar xzf httpd.tar.gz

mkdir apr
tar xzf apr.tar.gz

mkdir apr-util
tar xzf apr-util.tar.gz 

mkdir openssl
tar xzf openssl.tar.gz

mkdir zlib
tar xzf zlib.tar.gz 

unzip pcre.zip 

cp -r apr-1.6.5 $home/buildtemp/$httpd/srclib/apr/
cp -r apr-util-1.6.1 $home/buildtemp/$httpd/srclib/apr-util/


# build zib
cd $home/buildtemp/$ziplib
./configure --prefix=$zlibpath 
make
make install


# build openssl
cd $home/buildtemp/$ssllib
./config --debug --prefix=$sslpath  --openssldir=$sslpath/conf --with-zlib-include=$zlibpath/include  --with-zlib-lib=$zlibpath/lib zlib
make 
make install

#build pcre
echo "cd $home/buildtemp/$pcrelib"
cd $home/buildtemp/$pcrelib
./configure --prefix=$pcrepath
make 
make install


cd $home/buildtemp/$httpd
echo "cd $home/buildtemp/$httpd"
export LDFLAGS="-Wl,-rpath=$sslpath/lib"
echo "please run following cmd line by hand"
echo "./configure   --prefix=$dist --enable-debugger-mode --with-included-apr  --enable-modules=all --enable-deflate --with-z=$zlibpath --enable-rewrite=yes --enable-proxy_http=yes  --with-ssl=$sslpath --enable-ssl=yes --with-pcre=$pcrepath"

#./configure   --prefix=$dist --enable-debugger-mode --with-included-apr  --enable-modules=all --enable-deflate --with-z=$zlibpath --enable-rewrite=yes --enable-proxy_http=yes  --with-ssl=$sslpath --with-pcre=$pcrepath  --enable-ssl=yes
#make
#make install

# cd $home
# git clone https://github.com/infodog/fjt_xp.git
# cd $home/fjt_xp
# rm Makefile
# $dist/bin/apxs -n fjt -g
# make
# make install

##build fjt module的时候会碰到-Werror=的问题，只需要到/home/fjtv3ssl 目录下 grep -r Werror=就可以找到问题，修改一个配置文件就可以