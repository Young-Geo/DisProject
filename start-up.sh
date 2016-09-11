
#强制开启防火墙 端口
#sudo iptables -I INPUT -p tcp -m state --state NEW -m tcp --dport 22122 -j ACCEPT
#sudo iptables -I INPUT -p tcp -m state --state NEW -m tcp --dport 23000 -j ACCEPT
#sudo iptables -A INPUT -p tcp --dport 3306 -j ACCEPT

#spawn-fcgi -a 127.0.0.1 -p 8082 -f ./test/demo
#启动本地tracker
sudo /usr/bin/fdfs_trackerd conf/tracker.conf restart
#sudo /usr/bin/fdfs_trackerd /etc/fdfs/tracker.conf restart
#启动本地storage
sudo /usr/bin/fdfs_storaged conf/storage.conf restart
#sudo /usr/bin/fdfs_storaged /etc/fdfs/storage.conf restart
upid=`ps -aux | grep "./test/upload" | grep -v grep | head -n 1 | awk '{print $2}'`
kill -9 $upid
upid=`ps -aux | grep "./test/data" | grep -v grep | head -n 1 | awk '{print $2}'`
kill -9 $upid
spawn-fcgi -a 127.0.0.1 -p 8082 -f ./test/upload
spawn-fcgi -a 127.0.0.1 -p 8083 -f ./test/data
#spawn-fcgi -a 127.0.0.1 -p 8082 -f ./test/demo
sudo /usr/local/nginx/sbin/nginx -s reload
