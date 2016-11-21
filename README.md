Lisa Fan & Tyler Frasca
lfan01   & tfrasc01

Process Data Structure:
	-We created a data structure called Process to keep track of processes.
	
	-Each Process has a pid, arrival time, service time, priority (as a nice value), and runtime
	
	-Other values related to Processes, such as the Linux weight, virtual runtime, and timeslice are not part of the Process struct. We decided on this design, since these values are specific to the CFS algorithm, and would likely be tracked by the kernel, and not the Process itself.
	
	-When we read in the processes from the input file, we store them in an array of Processes, and sort that array by arrival time. If some processes have the same arrival time, we also sort by priority. This is to simplify things for the scheduler algorithms, as well as to better simulate actual events.


Traditional Algorithm:
	-We chose the FCFS (First Come First Serve) algorithm for our traditional scheduler, because we believed this would be the easiest to implement. This algorithm only requires you to keep track of arrival time, and each process runs to completion once it starts processing. We also believed that FCFS would be the most obvious baseline to compare the CFS to.
	
	-Our ready queue is implemented as an array of Processes, and operates similar to the Consumer/Producer problem. One pointer keeps track of the next empty index in the queue, and another keeps track of the next Process to "run". If the second pointer catches up to the first pointer, it waits until another Process arrives and enters the ready queue.
	
	-The priority of each process is stored in the struct, but not used by the scheduler.
	
	-At each timestep, the following things happen:
		1. If new processes have arrived, they are added to the ready queue.
	
		2. If no process is currently running, one is taken off the ready queue.
			-If the ready queue is empty, the scheduler prints an idle message and continues to the next timestep.
	
		3. The runtime on the current process, and the CPU usage time are incremented.
	
		4. If the current process has run for as long as its service time, the sum of the TAT and NTAT are updated, and we say that no process is currently running.
	
	-The above loop continues until all the processes have finished running. We check this by keeping track of the total number of processes in the input file, and the number of processes that have finished running.
	
	-After the loop, we print the total CPU usage percentage, calculated by dividing the CPU usage time by end time, and the average TAT and NTAT, calculated by dividing the sums by the number of processes.


Linux CFS:
	-Our ready queue is implemented as a red-black tree (see Extra Credit)
	
	-At each timestep, the following things happen:
		1. If new processes have arrived, they are added to the ready queue, and their weights are added to the sum of weights for all Processes on the ready queue (used to calculate timeslice)
	
		2. If no process is currently running, one is taken off the ready queue.
			-If the ready queue is empty, the scheduler prints an idle message and continues to the next timestep.
			-If there is a process on the ready queue, its timeslice is calculated.
	
		3. The runtime on the current process, and the CPU usage time are incremented. Two separate runtimes exist for the process: the total runtime for the process, and the current runtime since the process most recently got CPU access.
	
		4. If the current process has run for as long as its service time, the sum of the TAT and NTAT are updated, the weight of the process is taken off the sum of weights in the ready queue, and we say that no process is currently running.
	
		5. If the current process has run for as long as its timeslice, its virtual runtime is updated, the process is re-inserted into the ready queue, and we say that no process is currently running.
	
	-The above loop continues until all the processes have finished running. We check this by keeping track of the total number of processes in the input file, and the number of processes that have finished running.
	
	-After the loop, we print the total CPU usage percentage, calculated by dividing the CPU usage time by end time, and the average TAT and NTAT, calculated by dividing the sums by the number of processes.
	
	-Apart from increments and simple sums, there are two significant calculations in the CFS algorithm:
		-Timeslice: The actual equation for calculating timeslice is (target latency * process weight) / sum of ready weights. We decided to store timeslice as an integer, and not a float, since the timeslice is used to calculate how many timesteps a process is to run for. However,this means that there are cases when the timeslice is 0 using the above equation, particularly when the target latency is small, the process weight is small, and/or the sum of weights is large. In order to avoid this, we decided to always round up the calculation: ((target latency * process weight) + ready weights - 1) / ready weights.
	
		-Virtual Runtime: We calculate the virtual runtime as follows: process total runtime * (weight of nice value=0 / process weight). We decided to store the virtual runtime as a float, since otherwise any process with a nice value less than 0 would have a virtual runtime of 0.
	
	-Because all processes start with a virtual runtime of zero, our CFS algorithm chooses amongst these processes in the following way:
		1. If multiple processes arrive at different times while the CPU is busy, the process that arrived first gets chosen next
	
		2. If multiple processes arrive at the same time while the CPU is busy, the process with the higher priority gets chosen next
		This ordering is established both by the compare function for the red-black tree, and the compare function that sorts the processes from the input file
	
	-Target Latency chosen and why


Test Files:
	-When we validate the input file, we check the following things:
		-each line has 4 integer values
		-the pid and arrival time are at least 0
		-the service time is at least 1
		-the priority is between -20 and 19
		-the pids are unique

	input1.txt:

	input2.txt:

	processes.c:
		-In addition to our two input files, we also wrote a program called processes.c, that generates random processes.
		-To compile: make processes
		-To run: ./processes <number of processes> <output file name>
		-The pids of the processes will range from 0 to the number of processes specified, but they will appear in random order. The arrival time is randomized between 0 and 49, the service time between 1 and 20, and the priority between -20 and 19.


Extra Credit:
	-We implemented our ready queue for the CFS scheduler with a red-black tree. 
	
	-The source code for the red-black tree was taken from http://web.mit.edu/~emin/Desktop/ref_to_emin/www.old/source_code/red_black_tree/index.html, and all of the code taken is stored in the rb_tree directory.
	
	-Our ready queue stores virtual runtime as the key, and a pointer to its associated Process as the value for each node.
	
	-We wrote a compare function for sorting the red-black tree, which stores the node with the lowest virtual runtime in the leftmost position, and the node with the highest virtual runtime in the rightmost position. 
	
	-We were also required to declare a function to be passed to the remove node function. Because our values are pointers to Processes which may be used again, we do not need to free any memory, and so this function is blank.
