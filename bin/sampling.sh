###
### Use this shell script to create various execution and jobs data.
###
# Job: Empty Loop
sudo ./test_app 10000 1 1 99  #SCHED_FIFO with HIGH priority (from 10000 to 500000)
sleep 1
sudo ./test_app 10000 1 1 1   #SCHED_FIFO with LOW priority (from 10000 to 500000)
sleep 1
sudo ./test_app 10000 1 0 0   #SCHED_OTHER with no priority (from 10000 to 500000)
sleep 1
sudo ./test_app 510000 1 1 99 #SCHED_FIFO with HIGH priority (from 510000 to 1 Million)
sleep 1
sudo ./test_app 510000 1 1 1  #SCHED_FIFO with LOW priority (from 510000 to 1 Million)
sleep 1
sudo ./test_app 510000 1 0 0  #SCHED_OTHER with no priority (from 510000 to 1 Million))
sleep 1
# Job: Exchanging Variables
sudo ./test_app 10000 2 1 99  #SCHED_FIFO with HIGH priority (from 10000 to 500000)
sleep 1
sudo ./test_app 10000 2 1 1   #SCHED_FIFO with LOW priority (from 10000 to 500000)
sleep 1
sudo ./test_app 10000 2 0 0   #SCHED_OTHER with no priority (from 10000 to 500000)
sleep 1
sudo ./test_app 510000 2 1 99 #SCHED_FIFO with HIGH priority (from 510000 to 1 Million)
sleep 1
sudo ./test_app 510000 2 1 1  #SCHED_FIFO with LOW priority (from 510000 to 1 Million)
sleep 1
sudo ./test_app 510000 2 0 0  #SCHED_OTHER with no priority (from 510000 to 1 Million))
sleep 1
# Job: List Ordering
sudo ./test_app 10000 3 1 99  #SCHED_FIFO with HIGH priority (from 10000 to 500000)
sleep 1
sudo ./test_app 10000 3 1 1   #SCHED_FIFO with LOW priority (from 10000 to 500000)
sleep 1
sudo ./test_app 10000 3 0 0   #SCHED_OTHER with no priority (from 10000 to 500000)
sleep 1
sudo ./test_app 510000 3 1 99 #SCHED_FIFO with HIGH priority (from 510000 to 1 Million)
sleep 1
sudo ./test_app 510000 3 1 1  #SCHED_FIFO with LOW priority (from 510000 to 1 Million)
sleep 1
sudo ./test_app 510000 3 0 0  #SCHED_OTHER with no priority (from 510000 to 1 Million))
