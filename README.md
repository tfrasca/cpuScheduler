# cpuScheduler

-separate queues for processes vs. ready processes?

Process:
-pid
-arrival time
-service time
-runtime

-priority
-weight
-timeslice
-virtual runtime


Overall:
	Read in P's to structs in list (queue? red black tree?)
	Loop


FCFS:
while running_process || processes in queue:
	time++
	if !running_process:
		running_process = pop smallest arrival time

	if running_process.arrival_time > time:
		print "idle"
		pass

	running_process.runtime++
	cpu_used++

	if runtime == service time:
		TAT = time - arrival time
		sum_TAT += TAT
		sum_nTAT += TAT / service time
		print "finished"
		running_process = NULL
	else:
		print "running"


cpu usage = cpu_used / time
avgTAT = sum_TAT / num_p
avgnTAT = sum_nTAT / num_p


