###
### Use this shell script to create various execution and jobs data.
###
# Job: Empty Loop
sudo ./test_app --param 10000 --mode 1 --policy SCHED_FIFO --priority 99 --nowait   #SCHED_FIFO with HIGH priority (from 10000 to 500000)
sleep 1
sudo ./test_app --param 10000 --mode 1 --policy SCHED_FIFO --priority 1 --nowait    #SCHED_FIFO with LOW priority (from 10000 to 500000)
sleep 1
sudo ./test_app --param 10000 --mode 1 --policy SCHED_OTHER --priority 0 --nowait   #SCHED_OTHER with no priority (from 10000 to 500000)
sleep 1
sudo ./test_app --param 510000 --mode 1 --policy SCHED_FIFO --priority 99 --nowait  #SCHED_FIFO with HIGH priority (from 510000 to 1 Million)
sleep 1
sudo ./test_app --param 510000 --mode 1 --policy SCHED_FIFO --priority 1 --nowait   #SCHED_FIFO with LOW priority (from 510000 to 1 Million)
sleep 1
sudo ./test_app --param 510000 --mode 1 --policy SCHED_OTHER --priority 0 --nowait  #SCHED_OTHER with no priority (from 510000 to 1 Million)
sleep 1
sudo ./test_app --param 1010000 --jobs 100 --mode 1 --policy SCHED_FIFO --priority 99 --nowait  #SCHED_FIFO with HIGH priority (from 1010000 to 10 Million)
sleep 1
sudo ./test_app --param 1010000 --jobs 100 --mode 1 --policy SCHED_FIFO --priority 1 --nowait   #SCHED_FIFO with LOW priority (from 1010000 to 10 Million)
sleep 1
sudo ./test_app --param 1010000 --jobs 100 --mode 1 --policy SCHED_OTHER --priority 0 --nowait  #SCHED_OTHER with no priority (from 1010000 to 10 Million)
sleep 1

# Job: Exchanging Variables
sudo ./test_app --param 10000 --mode 2 --policy SCHED_FIFO --priority 99 --nowait    #SCHED_FIFO with HIGH priority (from 10000 to 500000)
sleep 1
sudo ./test_app --param 10000 --mode 2 --policy SCHED_FIFO --priority 1 --nowait     #SCHED_FIFO with LOW priority (from 10000 to 500000)
sleep 1
sudo ./test_app --param 10000 --mode 2 --policy SCHED_OTHER --priority 0 --nowait    #SCHED_OTHER with no priority (from 10000 to 500000)
sleep 1
sudo ./test_app --param 510000 --mode 2 --policy SCHED_FIFO --priority 99 --nowait   #SCHED_FIFO with HIGH priority (from 510000 to 1 Million)
sleep 1
sudo ./test_app --param 510000 --mode 2 --policy SCHED_FIFO --priority 1 --nowait    #SCHED_FIFO with LOW priority (from 510000 to 1 Million)
sleep 1
sudo ./test_app --param 510000 --mode 2 --policy SCHED_OTHER --priority 0 --nowait   #SCHED_OTHER with no priority (from 510000 to 1 Million)
sleep 1

# Job: List Ordering
sudo ./test_app --param 10000 --mode 3 --policy SCHED_FIFO --priority 99 --nowait    #SCHED_FIFO with HIGH priority (from 10000 to 500000)
sleep 1
sudo ./test_app --param 10000 --mode 3 --policy SCHED_FIFO --priority 1 --nowait     #SCHED_FIFO with LOW priority (from 10000 to 500000)
sleep 1
sudo ./test_app --param 10000 --mode 3 --policy SCHED_OTHER --priority 0 --nowait    #SCHED_OTHER with no priority (from 10000 to 500000)
sleep 1
sudo ./test_app --param 510000 --mode 3 --policy SCHED_FIFO --priority 99 --nowait   #SCHED_FIFO with HIGH priority (from 510000 to 1 Million)
sleep 1
sudo ./test_app --param 510000 --mode 3 --policy SCHED_FIFO --priority 1 --nowait    #SCHED_FIFO with LOW priority (from 510000 to 1 Million)
sleep 1
sudo ./test_app --param 510000 --mode 3 --policy SCHED_OTHER --priority 0 --nowait   #SCHED_OTHER with no priority (from 510000 to 1 Million)