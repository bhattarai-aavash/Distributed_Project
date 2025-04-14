gcc -o redis_writer redis_writer.c -lhiredis



hostname=$(hostname) 
echo "$hostname"
./redis_writer lhotse102 $hostname "1"